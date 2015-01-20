// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "config.h"
#include "modules/bluetooth/BluetoothDevice.h"

#include "public/platform/WebBluetoothDevice.h"
#include "wtf/OwnPtr.h"

namespace blink {

// TODO: REMOVE ON FIRST LAND?
BluetoothDevice::BluetoothDevice(const String& instanceId)
    : m_instanceId(instanceId)
    , m_deviceClass(0)
{
}

BluetoothDevice::BluetoothDevice(const String& instanceId,
                                 const String& name,
                                 const int32_t device_class)
    : m_instanceId(instanceId)
    , m_name(name)
    , m_deviceClass(device_class)
{
}

BluetoothDevice* BluetoothDevice::create(const WebBluetoothDevice& webDevice)
{
    return new BluetoothDevice(webDevice.instanceId,
                               webDevice.name,
                               webDevice.device_class);
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

} // namespace blink

