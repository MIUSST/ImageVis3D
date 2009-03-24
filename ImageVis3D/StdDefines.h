/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2008 Scientific Computing and Imaging Institute,
   University of Utah.


   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

/**
  \file    StdDefines.h
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \date    October 2008
*/

#pragma once

#ifndef STDDEFINES_H
#define STDDEFINES_H

#include "../Tuvok/StdTuvokDefines.h"

#define IV3D_VERSION 0.07
#define IV3D_VERSION_TYPE "beta"

#define HELP_URL  "http://www.sci.utah.edu/cibc/wiki/index.php/CIBC:Project:ImageVis3D#Manual"
#define TUTORIAL_URL  "http://www.sci.utah.edu/cibc/wiki/index.php/CIBC:Project:ImageVis3D#Video_Tutorials"
#define DEBUG_DUMP_SERVER   "ftp://ftp.sci.utah.edu"
#define DEBUG_DUMP_PATH     "upload/ImageVis3D/"
#define UPDATE_VERSION_PATH "http://software.sci.utah.edu/devbuilds/imagevis3d/"
#define UPDATE_NIGHTLY_PATH "http://software.sci.utah.edu/devbuilds/imagevis3d/"
#define UPDATE_STABLE_PATH  "http://software.sci.utah.edu/SCIdownload.php?item=imagevis3d"
#ifdef TUVOK_OS_WINDOWS
  #define UPDATE_FILE "ImageVis3D-Windows-Latest.zip"
  #define UPDATE_VERSION_FILE "Windows_Latest_Version.txt"
#endif

#ifdef TUVOK_OS_APPLE
  #define UPDATE_FILE "ImageVis3D-OSX-Latest.zip"
  #define UPDATE_VERSION_FILE "OSX_Latest_Version.txt"
#endif

#ifdef TUVOK_OS_LINUX
  #define UPDATE_FILE "ImageVis3D-Linux-Latest.zip"
  #define UPDATE_VERSION_FILE "Linux_Latest_Version.txt"
#endif

#endif // STDDEFINES_H
