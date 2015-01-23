// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BluetoothDevice_h
#define BluetoothDevice_h

#include "bindings/core/v8/ScriptWrappable.h"
#include "platform/heap/Heap.h"
#include "wtf/text/WTFString.h"

namespace blink {

class ScriptPromiseResolver;
struct WebBluetoothDevice;

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
    BluetoothDevice(const String& instanceId,
                    const String& name,
                    int32_t device_class,
                    uint16_t vendorId,
                    uint16_t productId,
                    uint16_t productVersion,
                    bool paired,
                    bool connected);

    static BluetoothDevice* create(const WebBluetoothDevice&);

    // Interface required by CallbackPromiseAdapter:
    typedef WebBluetoothDevice WebType;
    static BluetoothDevice* take(ScriptPromiseResolver*, WebBluetoothDevice*);
    static void dispose(blink::WebBluetoothDevice*);

    // Interface required by Garbage Collection:
    void trace(Visitor*) { }

    // IDL exposed interface:
    String instanceId() { return m_instanceId; }
    String name() { return m_name; }
    int32_t deviceClass(bool& isNull) { isNull = false; return m_deviceClass; }
    int16_t vendorId(bool& isNull) { isNull = false; return m_vendorId; }
    int16_t productId(bool& isNull) { isNull = false; return m_productId; }
    int16_t productVersion(bool& isNull) { isNull = false; return m_productVersion; }
    bool paired(bool& isNull) { isNull = false; return m_paired; }
    bool connected(bool& isNull) { isNull = false; return m_connected; }

private:
    const String m_instanceId;
    const String m_name;
    const int32_t m_deviceClass;
    const uint16_t m_vendorId;
    const uint16_t m_productId;
    const uint16_t m_productVersion;
    const bool m_paired;
    const bool m_connected;
};

} // namespace blink

#endif // BluetoothDevice_h
