# Based on original work by David Manura
# Copyright (C) 2007-2012 LuaDist.
# Copyright (C) 2013 Brian Sidebotham

# Redistribution and use of this file is allowed according to the terms of the
# MIT license.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

include_directories ( BEFORE SYSTEM ${openssl_BINARY_DIR} ../crypto .. ../include )

if( BUILD_SHARED_LIBS )
    add_definitions ( -DOPENSSL_BUILD_SHLIBSSL )
endif()

set ( LIBSRC s2_meth.c s2_srvr.c s2_clnt.c s2_lib.c s2_enc.c s2_pkt.c s3_meth.c s3_srvr.c
  s3_clnt.c s3_lib.c s3_enc.c s3_pkt.c s3_both.c s23_meth.c s23_srvr.c s23_clnt.c
  s23_lib.c s23_pkt.c t1_meth.c t1_srvr.c t1_clnt.c t1_lib.c t1_enc.c d1_meth.c d1_srvr.c
  d1_clnt.c d1_lib.c d1_pkt.c d1_both.c d1_enc.c ssl_lib.c ssl_err2.c ssl_cert.c
  ssl_sess.c ssl_ciph.c ssl_stat.c ssl_rsa.c ssl_asn1.c ssl_txt.c ssl_algs.c bio_ssl.c
  ssl_err.c kssl.c t1_reneg.c tls_srp.c s3_cbc.c d1_srtp.c )

set( LIBHDR ssl.h kssl.h ssl2.h ssl3.h tls1.h dtls1.h ssl23.h srtp.h)
file( COPY ${LIBHDR} DESTINATION ${openssl_BINARY_DIR}/openssl )

if(BUILD_OBJECT_LIBRARY_ONLY)
  add_library( ssl OBJECT ${LIBSRC} ${LIBHDR} )
else()
  add_library( ssl ${LIBSRC} )
  target_link_libraries( ssl crypto )

  #install( FILES ssl.h ssl2.h ssl3.h ssl23.h tls1.h dtls1.h kssl.h
  #    DESTINATION include/openssl )

  #install( TARGETS ssl
  #    RUNTIME DESTINATION bin
  #    LIBRARY DESTINATION lib
  #    ARCHIVE DESTINATION lib )
endif()

vs_add_to_grp(ssl "${VS_GRP_THIRD_PARTY}")
