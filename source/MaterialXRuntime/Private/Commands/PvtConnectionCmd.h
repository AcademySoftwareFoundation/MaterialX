//
// TM & (c) 2020 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_PVTMAKECONNECTIONCMD_H
#define MATERIALX_PVTMAKECONNECTIONCMD_H

#include <MaterialXRuntime/Private/PvtCommand.h>

#include <MaterialXRuntime/RtPort.h>

namespace MaterialX
{

class PvtConnectionCmd : public PvtCommand
{
  public:
    PvtConnectionCmd(const RtOutput& src, const RtInput& dest, ConnectionChange change) :
        _src(src),
        _dest(dest),
        _change(change)
    {}

    static PvtCommandPtr create(const RtOutput& src, const RtInput& dest, ConnectionChange change);

    void execute(RtCommandResult& result) override;
    void undo(RtCommandResult& result) override;

  private:
    void makeConnection(RtCommandResult& result);
    void breakConnection(RtCommandResult& result);
    
    virtual void updateConnectionProperties(const RtOutput& src, const RtInput& dest, RtCommandResult& result);

    RtOutput _src;
    RtInput _dest;
    ConnectionChange _change;
};

class PvtInterfaceConnectionCmd : public PvtConnectionCmd
{
  public:
    PvtInterfaceConnectionCmd(const RtOutput& src, const RtInput& dest, ConnectionChange change) :
        PvtConnectionCmd(src,dest,change)
    {}

    static PvtCommandPtr create(const RtOutput& src, const RtInput& dest, ConnectionChange change);

  private:
    void updateConnectionProperties(const RtOutput& src, const RtInput& dest, RtCommandResult& result) override;
};

}

#endif
