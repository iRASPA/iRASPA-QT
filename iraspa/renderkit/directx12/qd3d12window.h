/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtD3D12Window module
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QD3D12WINDOW_H
#define QD3D12WINDOW_H

#include <QAtomicInt>
#include <QPaintDeviceWindow>
#include <QImage>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include "rkrenderkitprotocols.h"

using namespace Microsoft::WRL;

QT_BEGIN_NAMESPACE

class QD3D12WindowPrivate;

class QD3D12Window : public QPaintDeviceWindow, public RKRenderViewController
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QD3D12Window)

public:
    struct Fence {
        Fence() : event(Q_NULLPTR) { }
        ~Fence();
        ComPtr<ID3D12Fence> fence;
        HANDLE event;
        QAtomicInt value;
    private:
        Q_DISABLE_COPY(Fence)
    };

    QD3D12Window(QWindow *parent = Q_NULLPTR);

    void setExtraRenderTargetCount(int count);
    virtual void initializeD3D();
    virtual void releaseD3D();
    virtual void resizeD3D(const QSize &size);
    virtual void paintD3D();
    virtual void afterPresent();

    ID3D12Device *device() const;
    ID3D12CommandQueue *commandQueue() const;
    ID3D12CommandAllocator *commandAllocator() const;
    ID3D12CommandAllocator *bundleAllocator() const;

    Fence *createFence() const;
    void waitForGPU(Fence *f) const;

    void transitionResource(ID3D12Resource *resource, ID3D12GraphicsCommandList *commandList,
                            D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) const;
    void uavBarrier(ID3D12Resource *resource, ID3D12GraphicsCommandList *commandList) const;

    ID3D12Resource *backBufferRenderTarget() const;
    D3D12_CPU_DESCRIPTOR_HANDLE backBufferRenderTargetCPUHandle() const;

    D3D12_CPU_DESCRIPTOR_HANDLE depthStencilCPUHandle() const;

    D3D12_CPU_DESCRIPTOR_HANDLE extraRenderTargetCPUHandle(int idx) const;
    D3D12_CPU_DESCRIPTOR_HANDLE extraDepthStencilCPUHandle(int idx) const;

    ID3D12Resource *createExtraRenderTargetAndView(D3D12_CPU_DESCRIPTOR_HANDLE viewHandle,
                                                   const QSize &size,
                                                   const float *clearColor = Q_NULLPTR,
                                                   int samples = 0);
    ID3D12Resource *createExtraDepthStencilAndView(D3D12_CPU_DESCRIPTOR_HANDLE viewHandle,
                                                   const QSize &size,
                                                   int samples = 0);

    quint32 alignedCBSize(quint32 size) const;
    quint32 alignedTexturePitch(quint32 rowPitch) const;
    quint32 alignedTextureOffset(quint32 offset) const;

    QImage readbackRGBA8888(ID3D12Resource *rt, D3D12_RESOURCE_STATES rtState, ID3D12GraphicsCommandList *commandList);

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *) Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(QD3D12Window)
};

QT_END_NAMESPACE

#endif
