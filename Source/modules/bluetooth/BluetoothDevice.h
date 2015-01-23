// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BluetoothDevice_h
#define BluetoothDevice_h

#include "bindings/core/v8/ScriptWrappable.h"
#include "platform/heap/Heap.h"
#include "public/platform/WebBluetoothDevice.h"
#include "wtf/text/WTFString.h"

namespace blink {

class ScriptPromiseResolver;

// BluetoothDevice represents a physical bluetooth device in the DOM. See IDL.
//
// Callbacks providing WebBluetoothDevice objects are handled by
// CallbackPromiseAdapter templatized with this class. See this class's
// "Interface required by CallbackPromiseAdapter" section and the
// CallbackPromiseAdapter class comments.
class BluetoothDevice final
    : public GarbageCollectedFinalized<BluetoothDevice>
    , public ScriptWrappable {
    DEFINE_WRAPPERTYPEINFO();
public:
    BluetoothDevice(const WebBluetoothDevice&);

    static BluetoothDevice* create(const WebBluetoothDevice&);

    // Interface required by CallbackPromiseAdapter:
    typedef WebBluetoothDevice WebType;
    static BluetoothDevice* take(ScriptPromiseResolver*, WebBluetoothDevice*);
    static void dispose(blink::WebBluetoothDevice*);

    // Interface required by Garbage Collection:
    void trace(Visitor*) { }

    // IDL exposed interface:
    String instanceId() { return m_webDevice.instanceId; }
    String name() { return m_webDevice.name; }
    int32_t deviceClass(bool& isNull) { isNull = false; return m_webDevice.deviceClass; }
    int16_t vendorId(bool& isNull) { isNull = false; return m_webDevice.vendorId; }
    int16_t productId(bool& isNull) { isNull = false; return m_webDevice.productId; }
    int16_t productVersion(bool& isNull) { isNull = false; return m_webDevice.productVersion; }
    bool paired(bool& isNull) { isNull = false; return m_webDevice.paired; }
    bool connected(bool& isNull) { isNull = false; return m_webDevice.connected; }

private:
    WebBluetoothDevice m_webDevice;
};

} // namespace blink

#endif // BluetoothDevice_h
