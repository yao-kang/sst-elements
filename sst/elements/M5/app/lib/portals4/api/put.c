/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil; -*-
 * vim:expandtab:shiftwidth=4:tabstop=4:
 */
#include <portals4.h>
#include <ptl_internal_netIf.h>

int PtlPut(ptl_handle_md_t  md_handle,
           ptl_size_t       local_offset,
           ptl_size_t       length,
           ptl_ack_req_t    ack_req,
           ptl_process_t    target_id,
           ptl_pt_index_t   pt_index,
           ptl_match_bits_t match_bits,
           ptl_size_t       remote_offset,
           void *           user_ptr,
           ptl_hdr_data_t   hdr_data)
{
    ptl_handle_ni_t ni_handle;
    PtlNIHandle( md_handle, &ni_handle );

    const ptl_internal_handle_converter_t ni = { ni_handle };
    const ptl_internal_handle_converter_t md = { md_handle };

    struct PtlAPI* api = GetPtlAPI( ni );

    int retval = api->PtlPut( api, md.s.code,
            local_offset,
            length,
            ack_req,
            target_id,
            pt_index,
            match_bits,
            remote_offset,
            user_ptr,
            hdr_data );

    if ( retval < 0 ) return -retval;

    return PTL_OK;
}
