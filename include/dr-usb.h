/*
 * Data-Router
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact:  Hocheol Seo <hocheol.seo@samsung.com>
 *               Injun Yang <injun.yang@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */



#ifndef _DR_USB_H
#define  _DR_USB_H


/*
 * Output control lines.
 */

#define ACM_CTRL_DTR		0x01
#define ACM_CTRL_RTS		0x02


enum {
	DTR_OFF,
	DTR_ON
};


#define GS_CDC_NOTIFY_SERIAL_STATE _IOW('S', 1, int)


/** @internal
  *  This function initialises the USB interface
  * 
  *   @return       This function returns 0 if the USB has been initialised successfully or it returns -1 on failure  
  *
  */
int _init_usb(void);


/** @internal
  *  This function deinitialises the USB interface
  */
void _deinit_usb(void);


/** @internal
  *  This function is used to write data to the USB interface
  * 
  *   @param       [in]   buf              data to be written to USB 
  *   @param       [in]   buf_len    no of bytes of data to be written to USB 
  *   @return       This function returns the no of bytes written to the USB. 
  */
int _write_to_usb(char *buf, int buf_len);



/** @internal
  *  This function is used to send modem line state to Host PC
  *
  *  @param       [in] ctrl    status of control lines    
  *  @return       This function returns zero  if successful else returns -EIO
  *
  */
int _send_usb_line_state(int ctrl);


#endif				// _DR_USB_H
