/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil; -*-
 * vim:expandtab:shiftwidth=4:tabstop=4:
 */
#include <portals4.h>

int PtlTriggeredPut(ptl_handle_md_t     md_handle,
                    ptl_size_t          local_offset,
                    ptl_size_t          length,
                    ptl_ack_req_t       ack_req,
                    ptl_process_t       target_id,
                    ptl_pt_index_t      pt_index,
                    ptl_match_bits_t    match_bits,
                    ptl_size_t          remote_offset,
                    void *              user_ptr,
                    ptl_hdr_data_t      hdr_data,
                    ptl_handle_ct_t     trig_ct_handle,
                    ptl_size_t          threshold)
{
    return PTL_OK;
}

int PtlTriggeredGet(ptl_handle_md_t     md_handle,
                    ptl_size_t          local_offset,
                    ptl_size_t          length,
                    ptl_process_t       target_id,
                    ptl_pt_index_t      pt_index,
                    ptl_match_bits_t    match_bits,
                    ptl_size_t          remote_offset,
                    void *              user_ptr,
                    ptl_handle_ct_t     trig_ct_handle,
                    ptl_size_t          threshold)
{
    return PTL_OK;
}

int PtlTriggeredAtomic(ptl_handle_md_t  md_handle,
                       ptl_size_t       local_offset,
                       ptl_size_t       length,
                       ptl_ack_req_t    ack_req,
                       ptl_process_t    target_id,
                       ptl_pt_index_t   pt_index,
                       ptl_match_bits_t match_bits,
                       ptl_size_t       remote_offset,
                       void *           user_ptr,
                       ptl_hdr_data_t   hdr_data,
                       ptl_op_t         operation,
                       ptl_datatype_t   datatype,
                       ptl_handle_ct_t  trig_ct_handle,
                       ptl_size_t       threshold)
{
    return PTL_OK;
}

int PtlTriggeredFetchAtomic(ptl_handle_md_t     get_md_handle,
                            ptl_size_t          local_get_offset,
                            ptl_handle_md_t     put_md_handle,
                            ptl_size_t          local_put_offset,
                            ptl_size_t          length,
                            ptl_process_t       target_id,
                            ptl_pt_index_t      pt_index,
                            ptl_match_bits_t    match_bits,
                            ptl_size_t          remote_offset,
                            void *              user_ptr,
                            ptl_hdr_data_t      hdr_data,
                            ptl_op_t            operation,
                            ptl_datatype_t      datatype,
                            ptl_handle_ct_t     trig_ct_handle,
                            ptl_size_t          threshold)
{
    return PTL_OK;
}

int PtlTriggeredSwap(ptl_handle_md_t    get_md_handle,
                     ptl_size_t         local_get_offset,
                     ptl_handle_md_t    put_md_handle,
                     ptl_size_t         local_put_offset,
                     ptl_size_t         length,
                     ptl_process_t      target_id,
                     ptl_pt_index_t     pt_index,
                     ptl_match_bits_t   match_bits,
                     ptl_size_t         remote_offset,
                     void *             user_ptr,
                     ptl_hdr_data_t     hdr_data,
                     void *             operand,
                     ptl_op_t           operation,
                     ptl_datatype_t     datatype,
                     ptl_handle_ct_t    trig_ct_handle,
                     ptl_size_t         threshold)
{
    return PTL_OK;
}

int PtlTriggeredCTInc(ptl_handle_ct_t   ct_handle,
                      ptl_ct_event_t    increment,
                      ptl_handle_ct_t   trig_ct_handle,
                      ptl_size_t        threshold)
{
    return PTL_OK;
}

int PtlTriggeredCTSet(ptl_handle_ct_t   ct_handle,
                      ptl_ct_event_t    new_ct,
                      ptl_handle_ct_t   trig_ct_handle,
                      ptl_size_t        threshold)
{
    return PTL_OK;
}
