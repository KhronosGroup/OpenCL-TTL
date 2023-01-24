/*
 * TTL_trace_macros.h
 *
 * Copyright (c) 2023 Mobileye
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

/**
 * @file Redefine for cases where the function is define with __TTL_TRACE_FN
 *
 * Functions defined using the __TTL_TRACE_FN expect the __LINE__ number of the caller
 * when __TTL_DEBUG > 0. The macros in this file append __LINE__ in this situation.
 */

#if __TTL_DEBUG > 0

#define TTL_import_sub_tensor(...) TTL_import_sub_tensor(__VA_ARGS__, __LINE__)
#define TTL_import(...) TTL_import(__VA_ARGS__, __LINE__)
#define TTL_blocking_import(...) TTL_blocking_import(__VA_ARGS__, __LINE__)
#define TTL_step_buffering(...) TTL_step_buffering(__VA_ARGS__, __LINE__)

#define TTL_blocking_export(...) TTL_blocking_export(__VA_ARGS__, __LINE__)
#define TTL_step_buffering(...) TTL_step_buffering(__VA_ARGS__, __LINE__)

#define TTL_step_buffering(...) TTL_step_buffering(__VA_ARGS__, __LINE__)
#define TTL_step_buffering(...) TTL_step_buffering(__VA_ARGS__, __LINE__)

#define TTL_start_simplex_buffering(...) TTL_start_simplex_buffering(__VA_ARGS__, __LINE__)
#define TTL_finish_simplex_buffering(...) TTL_finish_simplex_buffering(__VA_ARGS__, __LINE__)

#define TTL_step_buffering(...) TTL_step_buffering(__VA_ARGS__, __LINE__)

#define TTL_start_import_double_buffering(...) TTL_start_import_double_buffering(__VA_ARGS__)
#define TTL_finish_import_double_buffering(...) TTL_finish_import_double_buffering(__VA_ARGS__, __LINE__)

#define TTL_start_export_double_buffering(...) TTL_start_export_double_buffering(__VA_ARGS__, __LINE__)
#define TTL_finish_export_double_buffering(...) TTL_finish_export_double_buffering(__VA_ARGS__, __LINE__)

#define TTL_start_duplex_buffering(...) TTL_start_duplex_buffering(__VA_ARGS__, __LINE__)
#define TTL_finish_duplex_buffering(...) TTL_finish_duplex_buffering(__VA_ARGS__, __LINE__)
#endif
