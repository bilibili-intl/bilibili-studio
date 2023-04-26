// Copyright (c) 2022 The Chromium Embedded Framework Authors. All rights
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
// $hash=ac30aa16fee147cd041b64db7f2743d578dc6384$
//

#include "libcef_dll/ctocpp/response_ctocpp.h"
#include "libcef_dll/transfer_util.h"

// STATIC METHODS - Body may be edited by hand.

NO_SANITIZE("cfi-icall") CefRefPtr<CefResponse> CefResponse::Create() {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_response_t* _retval = cef_response_create();

  // Return type: refptr_same
  return CefResponseCToCpp::Wrap(_retval);
}

// VIRTUAL METHODS - Body may be edited by hand.

NO_SANITIZE("cfi-icall") bool CefResponseCToCpp::IsReadOnly() {
  cef_response_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, is_read_only))
    return false;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int _retval = _struct->is_read_only(_struct);

  // Return type: bool
  return _retval ? true : false;
}

NO_SANITIZE("cfi-icall") cef_errorcode_t CefResponseCToCpp::GetError() {
  cef_response_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_error))
    return ERR_NONE;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_errorcode_t _retval = _struct->get_error(_struct);

  // Return type: simple
  return _retval;
}

NO_SANITIZE("cfi-icall")
void CefResponseCToCpp::SetError(cef_errorcode_t error) {
  cef_response_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, set_error))
    return;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->set_error(_struct, error);
}

NO_SANITIZE("cfi-icall") int CefResponseCToCpp::GetStatus() {
  cef_response_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_status))
    return 0;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int _retval = _struct->get_status(_struct);

  // Return type: simple
  return _retval;
}

NO_SANITIZE("cfi-icall") void CefResponseCToCpp::SetStatus(int status) {
  cef_response_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, set_status))
    return;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->set_status(_struct, status);
}

NO_SANITIZE("cfi-icall") CefString CefResponseCToCpp::GetStatusText() {
  cef_response_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_status_text))
    return CefString();

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_string_userfree_t _retval = _struct->get_status_text(_struct);

  // Return type: string
  CefString _retvalStr;
  _retvalStr.AttachToUserFree(_retval);
  return _retvalStr;
}

NO_SANITIZE("cfi-icall")
void CefResponseCToCpp::SetStatusText(const CefString& statusText) {
  cef_response_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, set_status_text))
    return;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Unverified params: statusText

  // Execute
  _struct->set_status_text(_struct, statusText.GetStruct());
}

NO_SANITIZE("cfi-icall") CefString CefResponseCToCpp::GetMimeType() {
  cef_response_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_mime_type))
    return CefString();

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_string_userfree_t _retval = _struct->get_mime_type(_struct);

  // Return type: string
  CefString _retvalStr;
  _retvalStr.AttachToUserFree(_retval);
  return _retvalStr;
}

NO_SANITIZE("cfi-icall")
void CefResponseCToCpp::SetMimeType(const CefString& mimeType) {
  cef_response_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, set_mime_type))
    return;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Unverified params: mimeType

  // Execute
  _struct->set_mime_type(_struct, mimeType.GetStruct());
}

NO_SANITIZE("cfi-icall") CefString CefResponseCToCpp::GetCharset() {
  cef_response_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_charset))
    return CefString();

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_string_userfree_t _retval = _struct->get_charset(_struct);

  // Return type: string
  CefString _retvalStr;
  _retvalStr.AttachToUserFree(_retval);
  return _retvalStr;
}

NO_SANITIZE("cfi-icall")
void CefResponseCToCpp::SetCharset(const CefString& charset) {
  cef_response_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, set_charset))
    return;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Unverified params: charset

  // Execute
  _struct->set_charset(_struct, charset.GetStruct());
}

NO_SANITIZE("cfi-icall")
CefString CefResponseCToCpp::GetHeaderByName(const CefString& name) {
  cef_response_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_header_by_name))
    return CefString();

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Verify param: name; type: string_byref_const
  DCHECK(!name.empty());
  if (name.empty())
    return CefString();

  // Execute
  cef_string_userfree_t _retval =
      _struct->get_header_by_name(_struct, name.GetStruct());

  // Return type: string
  CefString _retvalStr;
  _retvalStr.AttachToUserFree(_retval);
  return _retvalStr;
}

NO_SANITIZE("cfi-icall")
void CefResponseCToCpp::SetHeaderByName(const CefString& name,
                                        const CefString& value,
                                        bool overwrite) {
  cef_response_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, set_header_by_name))
    return;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Verify param: name; type: string_byref_const
  DCHECK(!name.empty());
  if (name.empty())
    return;
  // Unverified params: value

  // Execute
  _struct->set_header_by_name(_struct, name.GetStruct(), value.GetStruct(),
                              overwrite);
}

NO_SANITIZE("cfi-icall")
void CefResponseCToCpp::GetHeaderMap(HeaderMap& headerMap) {
  cef_response_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_header_map))
    return;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Translate param: headerMap; type: string_map_multi_byref
  cef_string_multimap_t headerMapMultimap = cef_string_multimap_alloc();
  DCHECK(headerMapMultimap);
  if (headerMapMultimap)
    transfer_string_multimap_contents(headerMap, headerMapMultimap);

  // Execute
  _struct->get_header_map(_struct, headerMapMultimap);

  // Restore param:headerMap; type: string_map_multi_byref
  if (headerMapMultimap) {
    headerMap.clear();
    transfer_string_multimap_contents(headerMapMultimap, headerMap);
    cef_string_multimap_free(headerMapMultimap);
  }
}

NO_SANITIZE("cfi-icall")
void CefResponseCToCpp::SetHeaderMap(const HeaderMap& headerMap) {
  cef_response_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, set_header_map))
    return;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Translate param: headerMap; type: string_map_multi_byref_const
  cef_string_multimap_t headerMapMultimap = cef_string_multimap_alloc();
  DCHECK(headerMapMultimap);
  if (headerMapMultimap)
    transfer_string_multimap_contents(headerMap, headerMapMultimap);

  // Execute
  _struct->set_header_map(_struct, headerMapMultimap);

  // Restore param:headerMap; type: string_map_multi_byref_const
  if (headerMapMultimap)
    cef_string_multimap_free(headerMapMultimap);
}

NO_SANITIZE("cfi-icall") CefString CefResponseCToCpp::GetURL() {
  cef_response_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_url))
    return CefString();

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_string_userfree_t _retval = _struct->get_url(_struct);

  // Return type: string
  CefString _retvalStr;
  _retvalStr.AttachToUserFree(_retval);
  return _retvalStr;
}

NO_SANITIZE("cfi-icall") void CefResponseCToCpp::SetURL(const CefString& url) {
  cef_response_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, set_url))
    return;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Unverified params: url

  // Execute
  _struct->set_url(_struct, url.GetStruct());
}

// CONSTRUCTOR - Do not edit by hand.

CefResponseCToCpp::CefResponseCToCpp() {}

// DESTRUCTOR - Do not edit by hand.

CefResponseCToCpp::~CefResponseCToCpp() {}

template <>
cef_response_t*
CefCToCppRefCounted<CefResponseCToCpp, CefResponse, cef_response_t>::
    UnwrapDerived(CefWrapperType type, CefResponse* c) {
  NOTREACHED() << "Unexpected class type: " << type;
  return nullptr;
}

template <>
CefWrapperType CefCToCppRefCounted<CefResponseCToCpp,
                                   CefResponse,
                                   cef_response_t>::kWrapperType = WT_RESPONSE;
