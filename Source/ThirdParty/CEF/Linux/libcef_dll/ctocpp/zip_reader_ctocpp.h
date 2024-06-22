// Copyright (c) 2024 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
//
// ---------------------------------------------------------------------------
//
// This file was generated by the CEF translator tool. If making changes by
// hand only do so within the body of existing method and function
// implementations. See the translator.README.txt file in the tools directory
// for more information.
//
// $hash=dabe033a06945fda9a2b61b7001df3e2cd219877$
//

#ifndef CEF_LIBCEF_DLL_CTOCPP_ZIP_READER_CTOCPP_H_
#define CEF_LIBCEF_DLL_CTOCPP_ZIP_READER_CTOCPP_H_
#pragma once

#if !defined(WRAPPING_CEF_SHARED)
#error This file can be included wrapper-side only
#endif

#include "include/capi/cef_zip_reader_capi.h"
#include "include/cef_zip_reader.h"
#include "libcef_dll/ctocpp/ctocpp_ref_counted.h"

// Wrap a C structure with a C++ class.
// This class may be instantiated and accessed wrapper-side only.
class CefZipReaderCToCpp : public CefCToCppRefCounted<CefZipReaderCToCpp,
                                                      CefZipReader,
                                                      cef_zip_reader_t> {
 public:
  CefZipReaderCToCpp();
  virtual ~CefZipReaderCToCpp();

  // CefZipReader methods.
  bool MoveToFirstFile() override;
  bool MoveToNextFile() override;
  bool MoveToFile(const CefString& fileName, bool caseSensitive) override;
  bool Close() override;
  CefString GetFileName() override;
  int64_t GetFileSize() override;
  CefBaseTime GetFileLastModified() override;
  bool OpenFile(const CefString& password) override;
  bool CloseFile() override;
  int ReadFile(void* buffer, size_t bufferSize) override;
  int64_t Tell() override;
  bool Eof() override;
};

#endif  // CEF_LIBCEF_DLL_CTOCPP_ZIP_READER_CTOCPP_H_
