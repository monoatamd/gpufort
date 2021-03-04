// This file was generated by gpufort
{# Jinja2 template for generating interface modules      #}
{# This template works with data structures of the form :#}
{# *-[includes:str]                                      #}
{#  -[kernels:dict]-cName:str                            #}
{#                 -[kernelArgs:dict]                    #}
{#                 -[kernelCallArgNames:str]             #}
{#                 -[interfaceArgs:dict]                 #}
{#                 -[reductions:dict]                    #}
{#                 -[cBody:str]                          #}
{#                 -[fBody:str]                          #}

{% for file in includes %}
#include "{{file}}"
{% endfor %}
#include "hip/math_functions.h"
#include <cstdio>
#include <iostream>
#include <algorithm>

{% include 'templates/preamble.template.cpp' %}

{%- macro make_block(kernel) -%}
{% set krnlPrefix = kernel.kernelName %}
{% set ifacePrefix = kernel.interfaceName %}
{% for blockDim in kernel.block %}  const int {{krnlPrefix}}_block{{blockDim.dim}} = {{blockDim.value}};
{% endfor %}
  dim3 block({{ kernel.blockDims | join(",") }});
{%- endmacro -%}

{%- macro make_grid(kernel) -%}
{% set krnlPrefix = kernel.kernelName %}
{% set ifacePrefix = kernel.interfaceName %}
{% for sizeDim in kernel.size %}  const int {{krnlPrefix}}_N{{sizeDim.dim}} = {{sizeDim.value}};
{% endfor %}

{% if kernel.grid|length > 0 %}
{% for gridDim in kernel.grid %}  const int {{krnlPrefix}}_grid{{gridDim.dim}} = {{gridDim.value}};
  dim3 grid({% for gridDim in kernel.grid -%}{{krnlPrefix}}_grid{{gridDim.dim}}{{ "," if not loop.last }}{%- endfor %});
{% endfor %}{% else %}
{% for blockDim in kernel.block %}  const int {{krnlPrefix}}_grid{{blockDim.dim}} = divideAndRoundUp( {{krnlPrefix}}_N{{blockDim.dim}}, {{krnlPrefix}}_block{{blockDim.dim}} );
{% endfor %}
  dim3 grid({% for blockDim in kernel.block -%}{{krnlPrefix}}_grid{{blockDim.dim}}{{ "," if not loop.last }}{%- endfor %});
{% endif %}
{%- endmacro -%}

{# REDUCTION MACROS #}

{%- macro reductions_prepare(kernel,star) -%}
{%- set krnlPrefix = kernel.kernelName -%}
{%- set ifacePrefix = kernel.interfaceName -%}
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

#define divideAndRoundUp(x,y) ((x)/(y) + ((x) % (y) != 0))

{% for kernel in kernels %}
{% set krnlPrefix = kernel.kernelName %}
{% set ifacePrefix = kernel.interfaceName %}

// BEGIN {{krnlPrefix}}
  /* Fortran original: 
{{kernel.fBody | indent(2, True)}}
  */
{{kernel.interfaceComment | indent(2, True)}}

__global__ void {{kernel.launchBounds}} {{krnlPrefix}}({{kernel.kernelArgs | join(",")}}) {
{% for def in kernel.macros %}{{def.expr}}
{% endfor %}
{% for var in kernel.kernelLocalVars %}{{var | indent(2, True)}};
{% endfor %}

{{kernel.cBody | indent(2, True)}}
}

extern "C" void {{ifacePrefix}}(dim3* grid, dim3* block, const int sharedMem, hipStream_t stream,{{kernel.interfaceArgs | join(",")}}) {
{{ reductions_prepare(kernel,"*") }}
  // launch kernel
  hipLaunchKernelGGL(({{krnlPrefix}}), *grid, *block, sharedMem, stream, {{kernel.kernelCallArgNames | join(",")}});
{{ reductions_finalize(kernel,"*") }}
}
{% if kernel.isLoopKernel %}extern "C" void {{ifacePrefix}}_auto(const int sharedMem, hipStream_t stream,{{kernel.interfaceArgs | join(",")}}) {
{{ make_block(kernel) }}
{{ make_grid(kernel) }}   
{{ reductions_prepare(kernel,"") }}
  // launch kernel
  hipLaunchKernelGGL(({{krnlPrefix}}), grid, block, sharedMem, stream, {{kernel.kernelCallArgNames | join(",")}});
{{ reductions_finalize(kernel,"") }}
}
{% endif %}// END {{krnlPrefix}}

 
{% endfor %}{# kernels #}
