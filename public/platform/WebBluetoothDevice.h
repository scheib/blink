// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebBluetoothDevice_h
#define WebBluetoothDevice_h

#include "WebString.h"

namespace blink {

// Information describing a Bluetooth device provided by the platform.
struct WebBluetoothDevice {
    enum class VendorIDSource {
        Unknown,
        Bluetooth,
        USB
    };

    // FIXME: Remove after http://crrev.com/XXXXXXXXXXXXX-Content-Side
    WebBluetoothDevice(const WebString& instanceId)
        : instanceId(instanceId)
        , deviceClass(0)
        , vendorIDSource(VendorIDSource::Unknown)
        , vendorID(0)
        , productId(0)
        , productVersion(0)
        , paired(false)
        , connected(false)
    {
    }

    WebBluetoothDevice(const WebString& instanceId,
                       const WebString& name,
                       int32_t deviceClass,
                       VendorIDSource vendorIDSource,
                       uint16_t vendorID,
                       uint16_t productId,
                       uint16_t productVersion,
                       bool paired,
                       bool connected)
        : instanceId(instanceId)
        , name(name)
        , deviceClass(deviceClass)
        , vendorIDSource(vendorIDSource)
        , vendorID(vendorID)
        , productId(productId)
        , productVersion(productVersion)
        , paired(paired)
        , connected(connected)
    {
    }

    // Members corresponding to BluetoothDevice attributes as specified in IDL.
    const WebString instanceId;
    const WebString name;
    const int32_t deviceClass;
    const VendorIDSource vendorIDSource;
    const uint16_t vendorID;
    const uint16_t productId;
    const uint16_t productVersion;
    const bool paired;
    const bool connected;
};

} // namespace blink

#endif // WebBluetoothDevice_h
