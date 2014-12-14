

/* Copyright (c) 2011, Peter Barrett  
**  
** Permission to use, copy, modify, and/or distribute this software for  
** any purpose with or without fee is hereby granted, provided that the  
** above copyright notice and this permission notice appear in all copies.  
** 
** THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL  
** WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED  
** WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR  
** BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES  
** OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,  
** WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,  
** ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS  
** SOFTWARE.  
*/

#include "USBAPI.h"

#if defined(USBCON)
#ifdef HID_ENABLED

//#define RAWHID_ENABLED

//Singletons for tracker
Tracker_ Tracker;


//================================================================================
//================================================================================

//	HID report descriptor

#define LSB(_x) ((_x) & 0xFF)
#define MSB(_x) ((_x) >> 8)

extern const u8 _hidReportDescriptor[] PROGMEM;
const u8 _hidReportDescriptor[] = {
//Head Tracker - 3 axis joystick, no buttons
    0x05, 0x01,		// Usage Page (Generic Desktop) /
    0x09, 0x04,		// Usage (Joystick) / 
    0xa1, 0x01,		// Collection (Application) 
    0x05, 0x01,			// Usage Page (Generic Desktop)
    0x09, 0x30,			// Usage (X) 
    0x09, 0x31,			// Usage (Y) 
    0x09, 0x32,			// Usage (Z) 
    0x75, 0x10,			// Report Size (16) 
    0x95, 0x03,			// Report Count (3) 
    0x16, 0x00, 0x80, 		// Logical Minimum (-32768)
    0x26, 0xff, 0x7f, 		// Logical Maximum (32767)
    0x81, 0x02, 		// Input (Data, Variable, Absolute) 
    0xc0, 		// End Collection 
};

extern const HIDDescriptor _hidInterface PROGMEM;
const HIDDescriptor _hidInterface =
{
	D_INTERFACE(HID_INTERFACE,1,3,0,0),
	D_HIDREPORT(sizeof(_hidReportDescriptor)),
	D_ENDPOINT(USB_ENDPOINT_IN (HID_ENDPOINT_INT),USB_ENDPOINT_TYPE_INTERRUPT,0x40,0x01)
};

//================================================================================
//================================================================================
//	Driver

u8 _hid_protocol = 1;
u8 _hid_idle = 1;

#define WEAK __attribute__ ((weak))

int WEAK HID_GetInterface(u8* interfaceNum)
{
	interfaceNum[0] += 1;	// uses 1
	return USB_SendControl(TRANSFER_PGM,&_hidInterface,sizeof(_hidInterface));
}

int WEAK HID_GetDescriptor(int i)
{
	return USB_SendControl(TRANSFER_PGM,_hidReportDescriptor,sizeof(_hidReportDescriptor));
}

void WEAK HID_SendReport(u8 id, const void* data, int len)
{
	if (id) USB_Send(HID_TX, &id, 1);
	USB_Send(HID_TX | TRANSFER_RELEASE,data,len);
}

bool WEAK HID_Setup(Setup& setup)
{
	u8 r = setup.bRequest;
	u8 requestType = setup.bmRequestType;
	if (REQUEST_DEVICETOHOST_CLASS_INTERFACE == requestType)
	{
		if (HID_GET_REPORT == r)
		{
			//HID_GetReport();
			return true;
		}
		if (HID_GET_PROTOCOL == r)
		{
			//Send8(_hid_protocol);	// TODO
			return true;
		}
	}
	
	if (REQUEST_HOSTTODEVICE_CLASS_INTERFACE == requestType)
	{
		if (HID_SET_PROTOCOL == r)
		{
			_hid_protocol = setup.wValueL;
			return true;
		}

		if (HID_SET_IDLE == r)
		{
			_hid_idle = setup.wValueL;
			return true;
		}
	}
	return false;
}


//================================================================================
//	Tracker
//  Usage: Tracker.setState(inputs go here)
//

Tracker_::Tracker_()
{
}


void Tracker_::setState(TrackState_t *trackerSt)
{
	int16_t data[3];
	
	data[0] = trackerSt->xAxis;		// X axis
	data[1] = trackerSt->yAxis;		// Y axis
	data[2] = trackerSt->zAxis;		// Z axis
	
	//HID_SendReport(Report number, array of values in same order as HID descriptor, length)
	HID_SendReport(0, data, sizeof(data));
}

#endif

#endif /* if defined(USBCON) */
