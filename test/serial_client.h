/*
* serial-client
*
* Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
* PROPRIETARY/CONFIDENTIAL
*
* Contact: Hocheol Seo <hocheol.seo@samsung.com>,
*          Injun Yang <injun.yang@samsung.com>,
*          Seungyoun Ju <sy39.ju@samsung.com>
*
* This software is the confidential and proprietary information of
* SAMSUNG ELECTRONICS ("Confidential Information"). You agree and acknowledge
* that this software is owned by Samsung and you shall not disclose
* such Confidential Information and shall use it only in accordance with
* the terms of the license agreement you entered into with SAMSUNG ELECTRONICS.
* SAMSUNG make no representations or warranties about the suitability of
* the software, either express or implied, including but not limited to
* the implied warranties of merchantability, fitness for a particular purpose,
* or non-infringement. SAMSUNG shall not be liable for any damages suffered by
* licensee arising out of or related to this software.
*/

#if !defined(__SERIAL_CLIENT_H__)
#define __SERIAL_CLIENT_H__


#include "dlog.h"
#undef LOG_TAG
#define LOG_TAG "SERIAL_CLIENT"
#define DBG(fmt, args...) SLOGD(fmt, ##args)
#define ERR(fmt, args...) SLOGE(fmt, ##args)


#endif /* __SERIAL_CLIENT_H__ */
