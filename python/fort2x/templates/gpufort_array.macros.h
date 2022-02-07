{# SPDX-License-Identifier: MIT                                                 #}
{# Copyright (c) 2021 Advanced Micro Devices, Inc. All rights reserved. #}
{# requires that the bound_args and bound_args_single_line macros exist #}
{% import "gpufort.macros.h" as gm %}
{%- macro render_gpufort_array_c_bindings(datatypes,max_rank) -%}
{# C side #}
extern "C" {
{% for rank in range(1,max_rank+1) %}
{% set rank_ub = rank+1 %}
{% set c_type = "char" %}
{% set c_prefix = "gpufort_array"+rank|string %}
{% for async_suffix in ["_async",""] %}
{% set is_async = async_suffix == "_async" %}
  __host__ hipError_t {{c_prefix}}_init{{async_suffix}} (
      gpufort::array{{rank}}<{{c_type}}>* array,
      int bytes_per_element,
      {{c_type}}* data_host,
      {{c_type}}* data_dev,
      int* sizes,
      int* lower_bounds,{{"
      hipStream_t stream," if is_async}}
      gpufort::AllocMode alloc_mode,
      gpufort::SyncMode sync_mode
  ) {
    return array->init{{async_suffix}}(
      bytes_per_element,
      data_host, data_dev,
      sizes, lower_bounds,
      {{"stream," if is_async}}alloc_mode, sync_mode
    );
  } 
  
  __host__ hipError_t {{c_prefix}}_destroy{{async_suffix}}(
      gpufort::array{{rank}}<{{c_type}}>* array{{",
      hipStream_t stream" if is_async}}
  ) {
    return array->destroy{{async_suffix}}({{"stream" if is_async}});
  } 
  
  __host__ hipError_t {{c_prefix}}_copy_to_buffer{{async_suffix}} (
      gpufort::array{{rank}}<{{c_type}}>* array,
      void* buffer,
      hipMemcpyKind memcpy_kind{{",
      hipStream_t stream" if is_async}}
  ) {
    return array->copy_to_buffer{{async_suffix}}(buffer,memcpy_kind{{",stream" if is_async}});
  } 
  
  __host__ hipError_t {{c_prefix}}_copy_from_buffer{{async_suffix}} (
      gpufort::array{{rank}}<{{c_type}}>* array,
      void* buffer,
      hipMemcpyKind memcpy_kind{{",
      hipStream_t stream" if is_async}}
  ) {
    return array->copy_from_buffer{{async_suffix}}(buffer,memcpy_kind{{",stream" if is_async}});
  } 
  
  __host__ hipError_t {{c_prefix}}_copy_to_host{{async_suffix}} (
      gpufort::array{{rank}}<{{c_type}}>* array{{",
      hipStream_t stream" if is_async}}
  ) {
    return array->copy_to_host{{async_suffix}}({{"stream" if is_async}});
  } 
  
  __host__ hipError_t {{c_prefix}}_copy_to_device{{async_suffix}} (
      gpufort::array{{rank}}<{{c_type}}>* array{{",
      hipStream_t stream" if is_async}}
  ) {
    return array->copy_to_device{{async_suffix}}({{"stream" if is_async}});
  } 
  
  __host__ hipError_t {{c_prefix}}_dec_num_refs{{async_suffix}}(
      gpufort::array{{rank}}<{{c_type}}>* array,
      bool destroy_if_zero_refs{{",
      hipStream_t stream" if is_async}}
  ) {
    array->num_refs -= 1;
    if ( destroy_if_zero_refs && array->num_refs == 0 ) {
      return array->destroy{{async_suffix}}({{"stream" if is_async}});
    } {
      return hipSuccess;
    }
  } 
{% endfor %}

{% for target in ["host","device"] %}
{% set is_host = target == "host" %}
    /**
     * Allocate a {{target}} buffer with
     * the same size as the data buffers
     * associated with this gpufort array.
     * \see num_data_bytes()
     * \param[inout] pointer to the buffer to allocate
{% if is_host %}
     * \param[in] pinned If the memory should be pinned (default=true)
     * \param[in] flags  Flags for the host memory allocation (default=0).
{% endif %}
     */
    __host__ hipError_t {{c_prefix}}_allocate_{{target}}_buffer(
      gpufort::array{{rank}}<{{c_type}}>* array,
      void** buffer{{",bool pinned=true,int flags=0" if is_host}}
    ) {
      return array->allocate_{{target}}_buffer(buffer{{",pinned,flags" if is_host}});
    }

    /**
     * Deallocate a {{target}} buffer
     * created via the allocate_{{target}}_buffer routine.
     * \see num_data_bytes(), allocate_{{target}}_buffer
     * \param[inout] the buffer to deallocte
{% if is_host %}
     * \param[in] pinned If the memory to deallocate is pinned [default=true]
{% endif %}
     */
    __host__ hipError_t {{c_prefix}}_deallocate_{{target}}_buffer(
        gpufort::array{{rank}}<{{c_type}}>* array,
        void* buffer{{",bool pinned=true" if is_host}}) {
      return array->deallocate_{{target}}_buffer(buffer{{",pinned" if is_host}});
    }
{% endfor %}{# target #}
     
    /**
     * \return Size of the array in dimension 'dim'.
     * \param[in] dim selected dimension: 1,...,{{rank}}
     */
    __host__ __forceinline__ int {{c_prefix}}_size(
        gpufort::array{{rank}}<{{c_type}}>* array,
        int dim) {
      return array->data.size(dim);
    }
    
    /**
     * \return Lower bound (inclusive) of the array in dimension 'dim'.
     * \param[in] dim selected dimension: 1,...,{{rank}}
     */
    __host__ __forceinline__ int {{c_prefix}}_lbound(
        gpufort::array{{rank}}<{{c_type}}>* array,
        int dim) {
      return array->data.lbound(dim);
    }
    
    /**
     * \return Upper bound (inclusive) of the array in dimension 'dim'.
     * \param[in] dim selected dimension: 1,...,{{rank}}
     */
    __host__ __forceinline__ int {{c_prefix}}_ubound(
        gpufort::array{{rank}}<{{c_type}}>* array,
        int dim) {
      return array->data.ubound(dim);
    }

{% for d in range(rank-1,0,-1) %}
    /**
     * Collapse the array by fixing {{rank-d}} indices.
     * \return A gpufort array of rank {{d}}.
     * \param[inout] result the collapsed array
     * \param[in]    array  the original array
     * \param[in]    i{{d+1}},...,i{{rank}} indices to fix.
     */
    __host__ void {{c_prefix}}_collapse_{{d}}(
        gpufort::array{{d}}<{{c_type}}>* result,
        gpufort::array{{rank}}<{{c_type}}>* array,
{% for e in range(d+1,rank_ub) %}
        const int i{{e}}{{"," if not loop.last}}
{% endfor %}
    ) {
      auto collapsed_array = 
        array->collapse( 
{{"" | indent(10,True)}}{% for e in range(d+1,rank_ub) %}i{{e}}{{"," if not loop.last}}{%- endfor %});
      result->copy(collapsed_array);
    }
{% endfor %}
  
  __host__ void {{c_prefix}}_inc_num_refs(
      gpufort::array{{rank}}<{{c_type}}>* array
  ) {
    array->num_refs += 1;
  } 
{% endfor %}
} // extern "C"
{%- endmacro -%}