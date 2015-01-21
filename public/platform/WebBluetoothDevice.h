// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebBluetoothDevice_h
#define WebBluetoothDevice_h

#include "WebString.h"

namespace blink {

// Information describing a Bluetooth device provided by the platform.
struct WebBluetoothDevice {
    // FIXME: Remove after http://crrev.com/XXXXXXXXXXXXX-Content-Side
    WebBluetoothDevice(const WebString& instanceId)
        : instanceId(instanceId)
        , device_class(0)
        , vendorId(0)
        , productId(0)
        , paired(false)
        , connected(false)
    {
    }

    WebBluetoothDevice(const WebString& instanceId,
                       const WebString& name,
                       int32_t device_class,
                       uint16_t vendorId,
                       uint16_t productId,
                       bool paired,
                       bool connected)
        : instanceId(instanceId)
        , name(name)
        , device_class(device_class)
        , vendorId(vendorId)
        , productId(productId)
        , paired(paired)
        , connected(connected)
    {
    }

    // Members corresponding to BluetoothDevice attributes as specified in IDL.
    const WebString instanceId;
    const WebString name;
    const int32_t device_class;
    const uint16_t vendorId;
    const uint16_t productId;
    const bool paired;
    const bool connected;
};

} // namespace blink

#endif // WebBluetoothDevice_h
