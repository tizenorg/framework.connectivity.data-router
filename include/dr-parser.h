/*
 * Data-Router
 * Copyright (c) 2012-2013 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */



#ifndef _DR_PARSER_H_
#define _DR_PARSER_H_

/* AT tokens */
enum {
	TOKEN_ERROR,
	ATZ_TOKEN,
	AT_OSP_TOKEN,
	AT_TIZEN_OSP_TOKEN,
	OTHER_TOKEN,
};


int _get_at_cmd_type(char *buf);


#endif