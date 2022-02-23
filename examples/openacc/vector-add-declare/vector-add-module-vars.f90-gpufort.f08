#ifdef _GPUFORT
! This file was generated by GPUFORT
#endif
! SPDX-License-Identifier: MIT
! Copyright (c) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
module datatypes
#ifdef _GPUFORT
  use gpufort_acc_runtime
  use iso_c_binding
#endif
  integer, parameter :: N = 1000
  integer,allocatable,dimension(:) :: x1
  integer,pointer,dimension(:)     :: x2
  integer                          :: y(N)
  !$acc declare create(x1,x2,y)
end module

program main
  ! begin of program

#ifdef _GPUFORT
  use gpufort_acc_runtime
  use iso_c_binding
#endif
  use datatypes
  implicit none
  integer :: i
#ifdef _GPUFORT
  integer(4) :: y_exact(N)
  type(c_ptr) :: dev_y_i_
  type(c_ptr) :: dev_x2_i_
  type(c_ptr) :: dev_x1_i_
  type(c_ptr) :: dev_x1
  type(c_ptr) :: dev_y
  type(c_ptr) :: dev_x2
  call gpufort_acc_enter_region(implicit_region=.true.)
  if (associated(x2)) dev_x2 = gpufort_acc_present(x2,module_var=.true.,or=gpufort_acc_event_create)
  dev_y = gpufort_acc_present(y,module_var=.true.,or=gpufort_acc_event_create)

  allocate(x1(N),x2(N))
  dev_x1 = gpufort_acc_present(x1,module_var=.true.,or=gpufort_acc_event_create)
  dev_x2 = gpufort_acc_present(x2,module_var=.true.,or=gpufort_acc_event_create)
#else
  integer(4) :: y_exact(N)

  allocate(x1(N),x2(N))
#endif

  y_exact = 4

#ifdef _GPUFORT
  call gpufort_acc_enter_region()
  dev_x1_i_ = gpufort_acc_present(x1(i),or=gpufort_acc_event_copy)
  dev_x2_i_ = gpufort_acc_present(x2(i),or=gpufort_acc_event_copy)
  dev_y_i_ = gpufort_acc_present(y(i),or=gpufort_acc_event_copy)
  ! extracted to HIP C++ file
  call launch_main_23_auto(0,c_null_ptr,)
  call gpufort_acc_wait()
  call gpufort_acc_exit_region()

  call gpufort_acc_enter_region()
  dev_y_i_ = gpufort_acc_present(y(i),or=gpufort_acc_event_copy)
  dev_x1_i_ = gpufort_acc_present(x1(i),or=gpufort_acc_event_copy)
  dev_x2_i_ = gpufort_acc_present(x2(i),or=gpufort_acc_event_copy)
  ! extracted to HIP C++ file
  call launch_main_30_auto(0,c_null_ptr,)
  call gpufort_acc_wait()
  call gpufort_acc_exit_region()

  call gpufort_acc_update_host(y)
#else
  !$acc parallel loop
  do i = 1, N
    x1(i) = 1
    x2(i) = 1
    y(i) = 2
  end do

  !$acc parallel loop
  do i = 1, N
    y(i) = x1(i) + x2(i) + y(i)
  end do

  !$acc update host(y)
#endif

  do i = 1, N
    if ( y_exact(i) .ne.&
            y(i) ) ERROR STOP "GPU and CPU result do not match"
  end do

  print *, "PASSED"

#ifdef _GPUFORT
  call gpufort_acc_delete(x1,module_var=.true.)
  call gpufort_acc_delete(x2,module_var=.true.)
  deallocate(x1,x2)

call gpufort_acc_exit_region(implicit_region=.true.)
end program
#else
  deallocate(x1,x2)

end program
#endif