/*
 * TTL_pipeline_schemes.h
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

#include "TTL_core.h"
#include "TTL_import_export.h"
#include TTL_IMPORT_EXPORT_INCLUDE_H

#define TYPES_INCLUDE_FILE "pipelines/TTL_double_scheme.h"
#include "TTL_create_types.h"

#define TYPES_INCLUDE_FILE "pipelines/TTL_simplex_scheme.h"
#include "TTL_create_types.h"

#define TYPES_INCLUDE_FILE "pipelines/TTL_duplex_scheme.h"
#include "TTL_create_types.h"
