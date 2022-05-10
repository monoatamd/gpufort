{# SPDX-License-Identifier: MIT                                              #}
{# Copyright (c) 2020-2022 Advanced Micro Devices, Inc. All rights reserved. #}
{########################################################################################}
{% import "gpufort_array.macros.h" as gam %}
{########################################################################################}
{{ gam.render_header_begin("GPUFORT_ARRAY_H") }}
#include "gpufort_array_ptr.h"

{{ gam.render_gpufort_arrays(max_rank) }}
{{ gam.render_header_end("GPUFORT_ARRAY_H") }}
