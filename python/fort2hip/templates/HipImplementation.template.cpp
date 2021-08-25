{# SPDX-License-Identifier: MIT                                                 #}
{# Copyright (c) 2021 GPUFORT Advanced Micro Devices, Inc. All rights reserved. #}
// This file was generated by gpufort
{# Jinja2 template for generating interface modules      #}
{# This template works with data structures of the form :#}
{# *-[includes:str]                                      #}
{#  -[kernels:dict]-c_name:str                           #}
{#                 -[kernel_args:dict]                   #}
{#                 -[kernel_call_arg_names:str]          #}
{#                 -[interface_args:dict]                #}
{#                 -[reductions:dict]                    #}
{#                 -[c_body:str]                         #}
{#                 -[f_body:str]                         #}
#ifndef {{ guard }}
#define {{ guard }}
{% for file in includes %}
#include "{{file}}"
{% endfor %}
#include "hip/math_functions.h"
#include <cstdio>
#include <iostream>
#include <algorithm>
#include "gpufort.h"
{% if have_reductions -%}
#include "gpufort_reductions.h"
{%- endif -%}
{%- macro make_block(kernel) -%}
{% set krnl_prefix = kernel.kernel_name %}
{% set iface_prefix = kernel.interface_name %}
{% for block_dim in kernel.block %}  const int {{krnl_prefix}}_block{{block_dim.dim}} = {{block_dim.value}};
{% endfor %}
  dim3 block({{ kernel.block_dims | join(",") }});
{%- endmacro -%}
{%- macro make_grid(kernel) -%}
{% set krnl_prefix = kernel.kernel_name %}
{% set iface_prefix = kernel.interface_name %}
{% for size_dim in kernel.size %}  const int {{krnl_prefix}}_N{{size_dim.dim}} = {{size_dim.value}};
{% endfor %}
{% if kernel.grid|length > 0 %}
{% for grid_dim in kernel.grid %}  const int {{krnl_prefix}}_grid{{grid_dim.dim}} = {{grid_dim.value}};
  dim3 grid({% for grid_dim in kernel.grid -%}{{krnl_prefix}}_grid{{grid_dim.dim}}{{ "," if not loop.last }}{%- endfor %});
{% endfor %}{% else %}
{% for block_dim in kernel.block %}  const int {{krnl_prefix}}_grid{{block_dim.dim}} = divide_and_round_up( {{krnl_prefix}}_N{{block_dim.dim}}, {{krnl_prefix}}_block{{block_dim.dim}} );
{% endfor %}
  dim3 grid({% for block_dim in kernel.block -%}{{krnl_prefix}}_grid{{block_dim.dim}}{{ "," if not loop.last }}{%- endfor %});
{% endif %}
{%- endmacro -%}
{%- macro synchronize(krnl_prefix) -%}
  #if defined(SYNCHRONIZE_ALL) || defined(SYNCHRONIZE_{{krnl_prefix}})
  HIP_CHECK(hipStreamSynchronize(stream));
  #elif defined(SYNCHRONIZE_DEVICE_ALL) || defined(SYNCHRONIZE_DEVICE_{{krnl_prefix}})
  HIP_CHECK(hipDeviceSynchronize());
  #endif
{%- endmacro -%}
{%- macro print_array(krnl_prefix,inout,print_values,print_norms,array,rank) -%}
  GPUFORT_PRINT_ARRAY{{rank}}("{{krnl_prefix}}:{{inout}}:",{{print_values}},{{print_norms}},{{array}},
    {%- for i in range(1,rank+1) -%}{{array}}_n{{i}},{%- endfor -%}
    {%- for i in range(1,rank+1) -%}{{array}}_lb{{i}}{{"," if not loop.last}}{%- endfor -%});
{%- endmacro -%}
{# REDUCTION MACROS #}
{%- macro reductions_prepare(kernel,star) -%}
{%- set krnl_prefix = kernel.kernel_name -%}
{%- set iface_prefix = kernel.interface_name -%}
{%- if kernel.reductions|length > 0 -%}
{%- for var in kernel.reductions %} 
  {{ var.type }}* {{ var.buffer }};
  HIP_CHECK(hipMalloc((void **)&{{ var.buffer }}, __total_threads(({{star}}grid),({{star}}block)) * sizeof({{ var.type }} )));
{% endfor -%}
{%- endif -%}
{%- endmacro -%}
{%- macro reductions_finalize(kernel,star) -%}
{% if kernel.reductions|length > 0 -%}
{%- for var in kernel.reductions -%} 
  reduce<{{ var.type }}, reduce_op_{{ var.op }}>({{ var.buffer }}, __total_threads(({{star}}grid),({{star}}block)), {{ var.name }});
  HIP_CHECK(hipFree({{ var.buffer }}));
{% endfor -%}
{%- endif -%}
{%- endmacro %}
{% for kernel in kernels %}
{% set krnl_prefix = kernel.kernel_name %}
{% set iface_prefix = kernel.interface_name %}
// BEGIN {{krnl_prefix}}
/*
   HIP C++ implementation of the function/loop body of:

{{kernel.f_body | indent(3, True)}}
*/
{{kernel.interface_comment if (kernel.interface_comment|length>0)}}
{{kernel.modifier}} {{kernel.return_type}} {{kernel.launch_bounds}} {{krnl_prefix}}(
{% for arg in kernel.kernel_args %}
{{ arg | indent(4,True) }}{{"," if not loop.last else ") {"}}
{% endfor -%}
{% for def in kernel.macros %}{{def.expr | indent(2,True) }}
{% endfor %}
{% for var in kernel.kernel_local_vars %}{{var | indent(2, True)}};
{% endfor %}
{{kernel.c_body | indent(2, True)}}
}
{% if kernel.generate_launcher -%}
extern "C" void {{iface_prefix}}(
    dim3* grid,
    dim3* block,  
    const int sharedmem, 
    hipStream_t stream{{"," if (kernel.interface_args|length > 0) else ") {"}}
{% for arg in kernel.interface_args %}
{{ arg | indent(4,True) }}{{"," if not loop.last else ") {"}}
{% endfor -%}
{{ reductions_prepare(kernel,"*") }}{% if kernel.generate_debug_code %}
  #if defined(GPUFORT_PRINT_KERNEL_ARGS_ALL) || defined(GPUFORT_PRINT_KERNEL_ARGS_{{krnl_prefix}})
  std::cout << "{{krnl_prefix}}:gpu:args:";
  GPUFORT_PRINT_ARGS((*grid).x,(*grid).y,(*grid).z,(*block).x,(*block).y,(*block).z,sharedmem,stream,{{kernel.kernel_call_arg_names | join(",")}});
  #endif
  #if defined(GPUFORT_PRINT_INPUT_ARRAYS_ALL) || defined(GPUFORT_PRINT_INPUT_ARRAYS_{{krnl_prefix}})
  {% for array in kernel.input_arrays %}
  {{ print_array(krnl_prefix+":gpu","in","true","true",array.name,array.rank) }}
  {% endfor %}
  #elif defined(GPUFORT_PRINT_INPUT_ARRAY_NORMS_ALL) || defined(GPUFORT_PRINT_INPUT_ARRAY_NORMS_{{krnl_prefix}})
  {% for array in kernel.input_arrays %}
  {{ print_array(krnl_prefix+":gpu","in","false","true",array.name,array.rank) }}
  {% endfor %}
  #endif{% endif +%}
  // launch kernel
  hipLaunchKernelGGL(({{krnl_prefix}}), *grid, *block, sharedmem, stream, {{kernel.kernel_call_arg_names | join(",")}});
{{ reductions_finalize(kernel,"*") }}
{% if kernel.generate_debug_code %}
  {{ synchronize(krnl_prefix) }}
  #if defined(GPUFORT_PRINT_OUTPUT_ARRAYS_ALL) || defined(GPUFORT_PRINT_OUTPUT_ARRAYS_{{krnl_prefix}})
  {% for array in kernel.output_arrays %}
  {{ print_array(krnl_prefix+":gpu","out","true","true",array.name,array.rank) }}
  {% endfor %}
  #elif defined(GPUFORT_PRINT_OUTPUT_ARRAY_NORMS_ALL) || defined(GPUFORT_PRINT_OUTPUT_ARRAY_NORMS_{{krnl_prefix}})
  {% for array in kernel.output_arrays %}
  {{ print_array(krnl_prefix+":gpu","out","false","true",array.name,array.rank) }}
  {% endfor %}
  #endif
{% endif %}
}
{% if kernel.is_loop_kernel %}
extern "C" void {{iface_prefix}}_auto(
    const int sharedmem, 
    hipStream_t stream{{"," if (kernel.interface_args|length > 0) else ") {"}}
{% for arg in kernel.interface_args %}
{{ arg | indent(4,True) }}{{"," if not loop.last else ") {"}}
{% endfor -%}
{{ make_block(kernel) }}
{{ make_grid(kernel) }}   
{{ reductions_prepare(kernel,"") }}{% if kernel.generate_debug_code %}
  #if defined(GPUFORT_PRINT_KERNEL_ARGS_ALL) || defined(GPUFORT_PRINT_KERNEL_ARGS_{{krnl_prefix}})
  std::cout << "{{krnl_prefix}}:gpu:args:";
  GPUFORT_PRINT_ARGS(grid.x,grid.y,grid.z,block.x,block.y,block.z,sharedmem,stream,{{kernel.kernel_call_arg_names | join(",")}});
  #endif
  #if defined(GPUFORT_PRINT_INPUT_ARRAYS_ALL) || defined(GPUFORT_PRINT_INPUT_ARRAYS_{{krnl_prefix}})
  {% for array in kernel.input_arrays %}
  {{ print_array(krnl_prefix+":gpu","in","true","true",array.name,array.rank) }}
  {% endfor %}
  #elif defined(GPUFORT_PRINT_INPUT_ARRAY_NORMS_ALL) || defined(GPUFORT_PRINT_INPUT_ARRAY_NORMS_{{krnl_prefix}})
  {% for array in kernel.input_arrays %}
  {{ print_array(krnl_prefix+":gpu","in","false","true",array.name,array.rank) }}
  {% endfor %}
  #endif{% endif +%}
  // launch kernel
  hipLaunchKernelGGL(({{krnl_prefix}}), grid, block, sharedmem, stream, {{kernel.kernel_call_arg_names | join(",")}});
{{ reductions_finalize(kernel,"") }}
{% if kernel.generate_debug_code %}
  {{ synchronize(krnl_prefix) }}
  #if defined(GPUFORT_PRINT_OUTPUT_ARRAYS_ALL) || defined(GPUFORT_PRINT_OUTPUT_ARRAYS_{{krnl_prefix}})
  {% for array in kernel.output_arrays %}
  {{ print_array(krnl_prefix+":gpu","out","true","true",array.name,array.rank) }}
  {% endfor %}
  #elif defined(GPUFORT_PRINT_OUTPUT_ARRAY_NORMS_ALL) || defined(GPUFORT_PRINT_OUTPUT_ARRAY_NORMS_{{krnl_prefix}})
  {% for array in kernel.output_arrays %}
  {{ print_array(krnl_prefix+":gpu","out","false","true",array.name,array.rank) }}
  {% endfor %}
  #endif
{% endif %}
}
{% endif %}
{% if kernel.generate_cpu_launcher -%}
extern "C" void {{iface_prefix}}_cpu1(
    const int sharedmem, 
    hipStream_t stream{{"," if (kernel.interface_args|length > 0) else ") {"}}
{% for arg in kernel.interface_args %}
{{ arg | indent(4,True) }}{{"," if not loop.last else ") {"}}
{% endfor -%}
extern "C" void {{iface_prefix}}_cpu(
    const int sharedmem, 
    hipStream_t stream{{"," if (kernel.interface_args|length > 0) else ") {"}}
{% for arg in kernel.interface_args %}
{{ arg | indent(4,True) }}{{"," if not loop.last else ") {"}}
{% endfor -%}
{% if kernel.generate_debug_code %}
  #if defined(GPUFORT_PRINT_KERNEL_ARGS_ALL) || defined(GPUFORT_PRINT_KERNEL_ARGS_{{krnl_prefix}})
  std::cout << "{{krnl_prefix}}:cpu:args:";
  GPUFORT_PRINT_ARGS(sharedmem,stream,{{kernel.cpu_kernel_call_arg_names | join(",")}});
  #endif
  #if defined(GPUFORT_PRINT_INPUT_ARRAYS_ALL) || defined(GPUFORT_PRINT_INPUT_ARRAYS_{{krnl_prefix}})
  {% for array in kernel.input_arrays %}
  {{ print_array(krnl_prefix+":cpu","in","true","true",array.name,array.rank) }}
  {% endfor %}
  #elif defined(GPUFORT_PRINT_INPUT_ARRAY_NORMS_ALL) || defined(GPUFORT_PRINT_INPUT_ARRAY_NORMS_{{krnl_prefix}})
  {% for array in kernel.input_arrays %}
  {{ print_array(krnl_prefix+":cpu","in","false","true",array.name,array.rank) }}
  {% endfor %}
  #endif{% endif +%}
  // launch kernel
  {{iface_prefix}}_cpu1(sharedmem, stream, {{kernel.cpu_kernel_call_arg_names | join(",")}});
{% if kernel.generate_debug_code %}
  #if defined(GPUFORT_PRINT_OUTPUT_ARRAYS_ALL) || defined(GPUFORT_PRINT_OUTPUT_ARRAYS_{{krnl_prefix}})
  {% for array in kernel.output_arrays %}
  {{ print_array(krnl_prefix+":cpu","out","true","true",array.name,array.rank) }}
  {% endfor %}
  #elif defined(GPUFORT_PRINT_OUTPUT_ARRAY_NORMS_ALL) || defined(GPUFORT_PRINT_OUTPUT_ARRAY_NORMS_{{krnl_prefix}})
  {% for array in kernel.output_arrays %}
  {{ print_array(krnl_prefix+":cpu","out","false","true",array.name,array.rank) }}
  {% endfor %}
  #endif
{% endif %}
}{% endif %}
{% endif +%}
// END {{krnl_prefix}}
{% endfor %}{# kernels #}
#endif // {{ guard }} 
