// Copyright 2016 The Fuchsia Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>

#include <magenta/syscalls.h>

// This example tests transfering message pipe handles through message pipes. To do so, it:
//   Creates two message pipes, A and B, with handles A0-A1 and B0-B1, respectively
//   Sends message "1" into A0
//   Sends A1 to B0
//   Sends message "2" into A0
//   Reads H from B1 (should receive A1 again, possibly with a new value)
//   Sends "3" into A0
//   Reads from H until empty. Should read "1", "2", "3" in that order.
int main(void)
{
    mx_handle_t A[2];
    A[0] = _magenta_message_pipe_create(&A[1]);
    if (A[0] < 0) {
        printf("failed to create message pipe A: %u\n", (mx_status_t)A[0]);
        return 1;
    }
    mx_handle_t B[2];
    B[0] = _magenta_message_pipe_create(&B[1]);
    if (B[0] < 0) {
        printf("failed to create message pipe B: %u\n", (mx_status_t)B[0]);
        return 1;
    }
    mx_status_t status = _magenta_message_write(A[0], "1", 1u, NULL, 0u, 0u);
    if (status != NO_ERROR) {
        printf("failed to write message \"1\" into A0: %u\n", status);
        return 1;
    }
    status = _magenta_message_write(B[0], NULL, 0u, &A[1], 1u, 0u);
    if (status != NO_ERROR) {
        printf("failed to write message with handle A[1]: %u\n", status);
        return 1;
    }
    A[1] = MX_HANDLE_INVALID;
    status = _magenta_message_write(A[0], "2", 1u, NULL, 0u, 0u);
    if (status != NO_ERROR) {
        printf("failed to write message \"2\" into A0: %u\n", status);
        return 1;
    }
    mx_handle_t H;
    uint32_t num_bytes = 0u;
    uint32_t num_handles = 1u;
    status = _magenta_message_read(B[1], NULL, &num_bytes, &H, &num_handles, 0u);
    if (status != NO_ERROR) {
        printf("failed to read message from B1: %u\n", status);
        return 1;
    }
    if (num_handles != 1u || H == MX_HANDLE_INVALID) {
        printf("failed to read actual handle value from B1\n");
        return 1;
    }
    status = _magenta_message_write(A[0], "3", 1u, NULL, 0u, 0u);
    if (status != NO_ERROR) {
        printf("failed to write message \"3\" into A0: %u\n", status);
        return 1;
    }
    for (int i=0; i < 3; ++i) {
        char buf[1];
        num_bytes = 1u;
        num_handles = 0u;
        status = _magenta_message_read(H, buf, &num_bytes, NULL, &num_handles, 0u);
        if (status != NO_ERROR) {
            printf("failed to read message from H: %u\n", status);
            return 1;
        }
        printf("read message: %c\n", buf[0]);
    }

    _magenta_handle_close(A[0]);
    _magenta_handle_close(B[0]);
    _magenta_handle_close(B[1]);
    _magenta_handle_close(H);
    return 0;
}