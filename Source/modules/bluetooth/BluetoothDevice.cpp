// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "config.h"
#include "modules/bluetooth/BluetoothDevice.h"

#include "public/platform/WebBluetoothDevice.h"
#include "wtf/OwnPtr.h"

namespace blink {

BluetoothDevice::BluetoothDevice(const String& instanceId,
                                 const String& name,
                                 int32_t device_class,
                                 uint16_t vendorId,
                                 uint16_t productId,
                                 uint16_t productVersion,
                                 bool paired,
                                 bool connected)
    : m_instanceId(instanceId)
    , m_name(name)
    , m_deviceClass(device_class)
    , m_vendorId(vendorId)
    , m_productId(productId)
    , m_productVersion(productVersion)
    , m_paired(paired)
    , m_connected(connected)
{
}

BluetoothDevice* BluetoothDevice::create(const WebBluetoothDevice& webDevice)
{
    return new BluetoothDevice(webDevice.instanceId,
                               webDevice.name,
                               webDevice.device_class,
                               webDevice.vendorId,
                               webDevice.productId,
                               webDevice.productVersion,
                               webDevice.paired,
                               webDevice.connected);
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

