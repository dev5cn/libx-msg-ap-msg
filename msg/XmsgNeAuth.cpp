/*
  Copyright 2019 www.dev5.cn, Inc. dev5@qq.com
 
  This file is part of X-MSG-IM.
 
  X-MSG-IM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  X-MSG-IM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU Affero General Public License
  along with X-MSG-IM.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "XmsgNeAuth.h"

XmsgNeAuth::XmsgNeAuth()
{

}

void XmsgNeAuth::handle(shared_ptr<XscChannel> channel, SptrXitp trans, shared_ptr<XmsgNeAuthReq> req)
{
	if (req->neg().empty() || req->cgt().empty() || req->salt().empty() || req->sign().empty())
	{
		trans->endDesc(RET_FORMAT_ERROR, "request format error: %s", req->ShortDebugString().c_str());
		return;
	}
	SptrCgt cgt = ChannelGlobalTitle::parse(req->cgt());
	if (cgt == nullptr)
	{
		trans->endDesc(RET_FORMAT_ERROR, "channel global title format error, req: %s", req->ShortDebugString().c_str());
		return;
	}
	shared_ptr<XmsgImApCfgXmsgNeN2hAddr> cfgNe = XmsgNeAuth::findXmsgNeCfg(req->cgt());
	if (cfgNe == nullptr)
	{
		trans->endDesc(RET_USR_OR_PASSWORD_ERROR, "cgt or password error");
		return;
	}
	if (Crypto::sha256ToHexStrLowerCase(cfgNe->cgt() + req->salt() + cfgNe->pwd()) != req->sign()) 
	{
		trans->endDesc(RET_FORBIDDEN, "sign error");
		return;
	}
	LOG_INFO("x-msg network element auth successful, req: %s", req->ShortDebugString().c_str())
	shared_ptr<XmsgNeUsrAp> nu(new XmsgNeUsrAp(req->neg(), req->cgt(), trans->channel, cfgNe->prefix()));
	trans->channel->setXscUsr(nu);
	if (req->ext().find("sub-x-msg-client-disc-event") != req->ext().end()) 
	{
		if (req->neg() != X_MSG_IM_HLR) 
		{
			LOG_FAULT("it`s a bug, only x-msg-im-hlr subscribe x-msg-im-client lost event operation unsupported")
			trans->endDesc(RET_FORBIDDEN, "subscribe x-msg-im-client lost event operation unsupported");
			return;
		}
		XmsgNeUsrMgrAp::instance()->addSubClientDiscEventGroup(nu);
	}
	auto old = XmsgNeUsrMgrAp::instance()->add(nu);
	shared_ptr<XmsgNeAuthRsp> rsp(new XmsgNeAuthRsp());
	rsp->set_cgt(XmsgApCfg::instance()->cfgPb->cgt());
	trans->end(rsp);
	if (old == nullptr)
		return;
	LOG_WARN("have old network element on line, we will kick it, ne: %s", old->toString().c_str())
	auto c = old->channel;
	c->future([c]
	{
		c->close();
	});
}

shared_ptr<XmsgImApCfgXmsgNeN2hAddr> XmsgNeAuth::findXmsgNeCfg(const string& cgt)
{
	for (int i = 0; i < XmsgApCfg::instance()->cfgPb->n2h_size(); ++i) 
	{
		auto ne = XmsgApCfg::instance()->cfgPb->n2h(i);
		if (ne.cgt() != cgt)
			continue;
		shared_ptr<XmsgImApCfgXmsgNeN2hAddr> cfgNe(new XmsgImApCfgXmsgNeN2hAddr());
		cfgNe->CopyFrom(ne);
		return cfgNe;
	}
	return nullptr;
}

XmsgNeAuth::~XmsgNeAuth()
{

}

