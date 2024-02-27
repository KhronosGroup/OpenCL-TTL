/* OpenCL built-in library: async_work_group_copy()

   Copyright (c) 2018 pocl developers

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

/**
 * @brief async_work_group_copy_3D3D if not supported by all OpenCL drivers
 *
 * This is an implementation that can be included by defining TTL_COPY_3D
 */
__attribute__((overloadable)) event_t async_work_group_copy_3D3D(
    __local void *const dst, size_t dst_offset, const __global void *const src, size_t src_offset, size_t num_bytes_per_element,
    size_t num_elements_per_line, size_t num_lines, size_t num_planes, size_t src_total_line_length,
    size_t src_total_plane_spacing, size_t dst_total_line_length, size_t dst_total_plane_spacing, event_t event) {
    for (size_t plane = 0; plane < num_planes; plane++) {
        const __global uchar *src_ptr =
            src + ((src_offset + (src_total_plane_spacing * plane)) * num_bytes_per_element);
        __local uchar *dst_ptr = dst + ((dst_offset + (dst_total_plane_spacing * plane)) * num_bytes_per_element);

        for (size_t line = 0; line < num_lines; line++) {
            async_work_group_copy(dst_ptr, src_ptr, num_bytes_per_element * num_elements_per_line, event);

            src_ptr += src_total_line_length * num_bytes_per_element;
            dst_ptr += dst_total_line_length * num_bytes_per_element;
        }
    }

    return event;
}

/**
 * @brief async_work_group_copy_3D3D if not supported by all OpenCL drivers
 *
 * This is an implementation that can be included by defining TTL_COPY_3D
 */
__attribute__((overloadable)) event_t async_work_group_copy_3D3D(
    __global void *const dst, size_t dst_offset, const __local void *const src, size_t src_offset, size_t num_bytes_per_element,
    size_t num_elements_per_line, size_t num_lines, size_t num_planes, size_t src_total_line_length,
    size_t src_total_plane_spacing, size_t dst_total_line_length, size_t dst_total_plane_spacing, event_t event) {
    for (size_t plane = 0; plane < num_planes; plane++) {
        const __local uchar *src_ptr = src + ((src_offset + (src_total_plane_spacing * plane)) * num_bytes_per_element);
        __global uchar *dst_ptr = dst + ((dst_offset + (dst_total_plane_spacing * plane)) * num_bytes_per_element);

        for (size_t line = 0; line < num_lines; line++) {
            async_work_group_copy(dst_ptr, src_ptr, num_bytes_per_element * num_elements_per_line, event);

            src_ptr += src_total_line_length * num_bytes_per_element;
            dst_ptr += dst_total_line_length * num_bytes_per_element;
        }
    }

    return event;
}
