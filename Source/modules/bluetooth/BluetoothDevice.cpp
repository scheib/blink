// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "config.h"
#include "modules/bluetooth/BluetoothDevice.h"

#include "wtf/OwnPtr.h"

namespace blink {

BluetoothDevice::BluetoothDevice(const WebBluetoothDevice& webDevice)
    : m_webDevice(webDevice)
{
}

BluetoothDevice* BluetoothDevice::create(const WebBluetoothDevice& webDevice)
{
    return new BluetoothDevice(webDevice);
}

BluetoothDevice* BluetoothDevice::take(ScriptPromiseResolver*, WebBluetoothDevice* webDeviceRawPointer)
{
    OwnPtr<WebBluetoothDevice> webDevice = adoptPtr(webDeviceRawPointer);
    return BluetoothDevice::create(*webDevice);
}

void BluetoothDevice::dispose(WebBluetoothDevice* webDeviceRaw)
{
    delete webDeviceRaw;
}

String BluetoothDevice::vendorIDSource() {
    switch (m_webDevice.vendorIDSource) {
        // FIXME: Should return undefined when Unknown. http://crbug.com/451604
        case WebBluetoothDevice::VendorIDSource::Unknown: return "";
        case WebBluetoothDevice::VendorIDSource::Bluetooth: return "bluetooth";
        case WebBluetoothDevice::VendorIDSource::USB: return "usb";
    }
    ASSERT_NOT_REACHED();
    return "";
}

} // namespace blink

