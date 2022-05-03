# SPDX-License-Identifier: MIT
# Copyright (c) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
import subprocess

from . import logging

def run_subprocess(cmd,verbose=False):
    """Run the subprocess in a blocking manner, collect error code,
    standard output and error output. 
    """
    logging.log_info("util.subprocess", "run_subprocess", " ".join(cmd))
    if verbose:
        print(" ".join(cmd))
     
    p = subprocess.Popen(cmd,
                         shell=False,
                         stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE)
    status = p.wait()
    return status, p.stdout.read().decode("utf-8"), p.stderr.read().decode(
        "utf-8")