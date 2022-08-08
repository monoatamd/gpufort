{# SPDX-License-Identifier: MIT #}
{# Copyright (c) 2020-2022 Advanced Micro Devices, Inc. All rights reserved. #}
{########################################################################################}
{%- set data_clauses = [
    "delete",
    "present",
    "no_create",
    "create",
    "copy",
    "copyin",
    "copyout",
  ]
-%}
{########################################################################################}
! SPDX-License-Identifier: MIT
! Copyright (c) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
module gpufortrt
  use gpufortrt_api_core
    
  !> Lookup device pointer for given host pointer.
  !> \param[in] condition condition that must be met, otherwise host pointer is returned. Defaults to '.true.'.
  !> \param[in] if_present Only return device pointer if one could be found for the host pointer.
  !>                       otherwise host pointer is returned. Defaults to '.false.'.
  !> \note Returns a c_null_ptr if the host pointer is invalid, i.e. not C associated.
{{ render_interface("gpufortrt_use_device",True) | indent(2,True) }}

{% for mapping in data_clauses %} 
{{ render_interface("gpufortrt_"+mapping,True) | indent(2,True) }}

{% endfor %}
{% for mapping in data_clauses %} 
{{ render_interface("gpufortrt_map_"+mapping,True) | indent(2,True) }}

{% endfor %}

  !> Update Directive
{{ render_interface("gpufortrt_update_self",True) | indent(2,True) }}

{{ render_interface("gpufortrt_update_device",True) | indent(2,True) }}

  contains

    !
    ! autogenerated routines for different inputs
    !

{% for tuple in datatypes -%}
{%-  for dims in dimensions -%}
{%     if dims > 0 %}
{%       set shape = ',shape(hostptr)' %}
{%       set size = 'size(hostptr)*' %}
{%       set rank = ',dimension(' + ':,'*(dims-1) + ':)' %}
{%     else %}
{%       set shape = '' %}
{%       set size = '' %}
{%       set rank = '' %}
{%     endif %}
{%     set suffix = tuple[0] + "_" + dims|string %}
    function gpufortrt_use_device_{{suffix}}(hostptr,lbounds,condition,if_present) result(resultptr)
      use iso_c_binding
      use gpufortrt_core, only: gpufortrt_use_device_b
      implicit none
      {{tuple[2]}},target{{ rank }},intent(in) :: hostptr
      integer(c_int),dimension({{ dims }}),intent(in),optional :: lbounds
      logical,intent(in),optional  :: condition,if_present
      !
      {{tuple[2]}},pointer{{ rank }} :: resultptr
      ! 
      type(c_ptr) :: cptr
      !
      cptr = gpufortrt_use_device_b(c_loc(hostptr),{{size}}{{tuple[1]}}_c_size_t,condition,if_present)
      !
      call c_f_pointer(cptr,resultptr{{shape}})
{%     if dims > 0 %}
      if ( present(lbounds) ) then
{{ render_set_fptr_lower_bound("resultptr","hostptr",dims) | indent(8,True) }}
      endif
{%     endif %}
    end function 

    function gpufortrt_present_{{suffix}}(hostptr) result(deviceptr)
      use iso_c_binding
      use gpufortrt_core, only: gpufortrt_present_b, gpufortrt_map_kind_undefined
      implicit none
      {{tuple[2]}},target{{ rank }},intent(in) :: hostptr
      !
      type(c_ptr) :: deviceptr
      !
      deviceptr = gpufortrt_present_b(c_loc(hostptr),{{size}}{{tuple[1]}}_c_size_t)
    end function

    function gpufortrt_create_{{suffix}}(hostptr) result(deviceptr)
      use iso_c_binding
      use gpufortrt_core, only: gpufortrt_create_b
      implicit none
      {{tuple[2]}},target{{ rank }},intent(in) :: hostptr
      !
      type(c_ptr) :: deviceptr
      !
      deviceptr = gpufortrt_create_b(c_loc(hostptr),{{size}}{{tuple[1]}}_c_size_t)
    end function

    function gpufortrt_no_create_{{suffix}}(hostptr) result(deviceptr)
      use iso_c_binding
      use gpufortrt_core, only: gpufortrt_no_create_b
      implicit none
      {{tuple[2]}},target{{ rank }},intent(in) :: hostptr
      !
      type(c_ptr) :: deviceptr
      !
      deviceptr = gpufortrt_no_create_b(c_loc(hostptr))
    end function

    subroutine gpufortrt_delete_{{suffix}}(hostptr,finalize)
      use iso_c_binding
      use gpufortrt_core, only: gpufortrt_delete_b
      implicit none
      {{tuple[2]}},target{{ rank }},intent(in) :: hostptr
      logical,intent(in),optional              :: finalize
      !
      call gpufortrt_delete_b(c_loc(hostptr),finalize)
    end subroutine

    function gpufortrt_copyin_{{suffix}}(hostptr,async) result(deviceptr)
      use iso_c_binding
      use gpufortrt_core, only: gpufortrt_copyin_b
      implicit none
      {{tuple[2]}},target{{ rank }},intent(in) :: hostptr
      integer,intent(in),optional :: async
      !
      type(c_ptr) :: deviceptr
      !
      deviceptr = gpufortrt_copyin_b(c_loc(hostptr),{{size}}{{tuple[1]}}_c_size_t,async)
    end function

    function gpufortrt_copyout_{{suffix}}(hostptr,async) result(deviceptr)
      use iso_c_binding
      use gpufortrt_core, only: gpufortrt_copyout_b
      implicit none
      {{tuple[2]}},target{{ rank }},intent(inout) :: hostptr
      integer,intent(in),optional :: async
      !
      type(c_ptr) :: deviceptr
      !
      deviceptr = gpufortrt_copyout_b(c_loc(hostptr),{{size}}{{tuple[1]}}_c_size_t,async)
    end function

    function gpufortrt_copy_{{suffix}}(hostptr,async) result(deviceptr)
      use iso_c_binding
      use gpufortrt_core, only: gpufortrt_copy_b
      implicit none
      {{tuple[2]}},target{{ rank }},intent(inout) :: hostptr
      integer,intent(in),optional :: async
      !
      type(c_ptr) :: deviceptr
      !
      deviceptr = gpufortrt_copy_b(c_loc(hostptr),{{size}}{{tuple[1]}}_c_size_t,async)
    end function

{% for target in ["host","device"] %}
    subroutine gpufortrt_update_{{target}}_{{suffix}}(hostptr,condition,if_present,async)
      use iso_c_binding
      use gpufortrt_core, only: gpufortrt_update_self_b
      implicit none
      {{tuple[2]}},target{{ rank }},intent(inout) :: hostptr
      logical,intent(in),optional :: condition, if_present
      integer,intent(in),optional :: async
      !
      call gpufortrt_update_{{target}}_b(c_loc(hostptr),condition,if_present,async)
    end subroutine
{% endfor %}
{% endfor %}
{% endfor %}

{{ render_map_routines(data_clauses,datatypes,dimensions) | indent(2,True) }}
end module
