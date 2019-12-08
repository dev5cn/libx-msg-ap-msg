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
#include <libx-msg-ap-core.h>
#include "XmsgApMsg.h"
#include "mgr/XmsgImMgrNeNetLoad.h"
#include "mgr/XmsgImMgrNeXscServerQuery.h"
#include "mgr/XmsgImMgrNeXscWorkerCount.h"
#include "msg/XmsgApClientKick.h"
#include "msg/XmsgImHlrAttachSimple.h"
#include "msg/XmsgNeAuth.h"

XmsgApMsg::XmsgApMsg()
{

}

void XmsgApMsg::init(vector<shared_ptr<XmsgImN2HMsgMgr>> pubMsgMgrs, shared_ptr<XmsgImN2HMsgMgr> priMsgMgr)
{
	for (auto& it : pubMsgMgrs) 
	{
		shared_ptr<XmsgImN2HMsgMgr> pubMsgMgr = it;
		X_MSG_N2H_PRPC_BEFOR_AUTH(pubMsgMgr, XmsgImHlrAttachSimpleReq, XmsgImHlrAttachSimpleRsp, XmsgImHlrAttachSimple::handle)
		pubMsgMgr->setItcp([](XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu)
		{
			auto clientChannel = static_pointer_cast<XscChannel>(channel->shared_from_this()); 
			if (pdu->transm.trans->msg == XmsgImHlrAttachSimpleReq::descriptor()->name()) 
			{
				if (pdu->transm.trans->trans == XSC_TAG_TRANS_BEGIN)
				{
					return XscMsgItcpRetType::DISABLE;
				}
				return XscMsgItcpRetType::EXCEPTION;
			}
			SptrClient client = static_pointer_cast<XmsgClient>(channel->usr.lock()); 
			return XmsgApMsg::pubMsgRoute(clientChannel, client, pdu);
		});
	}
	X_MSG_N2H_PRPC_BEFOR_AUTH(priMsgMgr, XmsgNeAuthReq, XmsgNeAuthRsp, XmsgNeAuth::handle)
	X_MSG_N2H_PRPC_AFTER_AUTH(priMsgMgr, XmsgApClientKickReq, XmsgApClientKickRsp, XmsgApClientKick::handle)
	X_MSG_N2H_PRPC_AFTER_AUTH(priMsgMgr, XmsgImMgrNeNetLoadReq, XmsgImMgrNeNetLoadRsp, XmsgImMgrNeNetLoad::handle)
	X_MSG_N2H_PRPC_AFTER_AUTH(priMsgMgr, XmsgImMgrNeXscServerQueryReq, XmsgImMgrNeXscServerQueryRsp, XmsgImMgrNeXscServerQuery::handle)
	X_MSG_N2H_PRPC_AFTER_AUTH(priMsgMgr, XmsgImMgrNeXscWorkerCountReq, XmsgImMgrNeXscWorkerCountRsp, XmsgImMgrNeXscWorkerCount::handle)
	priMsgMgr->setItcp([](XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu)
	{
		return XmsgApMsg::priMsgRoute(wk, channel, pdu);
	});
}

XscMsgItcpRetType XmsgApMsg::pubMsgRoute(shared_ptr<XscChannel> clientChannel , SptrClient client , shared_ptr<XscProtoPdu> pdu)
{
	if (pdu->transm.trans->trans == XSC_TAG_TRANS_BEGIN)
		return XmsgApMsg::pubMsgRoute4begin(clientChannel, client, pdu);
	if (pdu->transm.trans->trans == XSC_TAG_TRANS_END)
		return XmsgApMsg::pubMsgRoute4end(clientChannel, client, pdu);
	if (pdu->transm.trans->trans == XSC_TAG_TRANS_UNIDIRECTION)
		return XmsgApMsg::pubMsgRoute4unidirection(clientChannel, client, pdu);
	LOG_FAULT("it`s a bug, unexpected XSC_TAG_TRANS: %04X", pdu->transm.trans->trans)
	return XscMsgItcpRetType::EXCEPTION;
}

XscMsgItcpRetType XmsgApMsg::pubMsgRoute4begin(shared_ptr<XscChannel> clientChannel, shared_ptr<XmsgClient> client, shared_ptr<XscProtoPdu> pdu)
{
	if (client == nullptr || !client->isAttached())
	{
		LOG_DEBUG("no permission, message: %s, channel: %s", pdu->transm.trans->msg.c_str(), clientChannel->toString().c_str())
		XmsgApMsg::exceptionEnd(clientChannel, pdu, client, RET_FORBIDDEN, "need auth");
		return XscMsgItcpRetType::FORBIDDEN;
	}
	if (!client->local) 
	{
		LOG_FAULT("it`s a bug, current x-msg-ap version unsupported foreign attach, message: %s, channel: %s", pdu->transm.trans->msg.c_str(), clientChannel->toString().c_str())
		XmsgApMsg::exceptionEnd(clientChannel, pdu, client, RET_UNSUPPORTED, "unsupported foreign attach");
		return XscMsgItcpRetType::FORBIDDEN;
	}
	shared_ptr<XmsgNeUsrAp> nu = XmsgNeUsrMgrAp::instance()->findByMsgName(pdu->transm.trans->msg);
	if (nu == nullptr) 
	{
		LOG_WARN("can not found any network element to handle this message, may be all was lost, message: %s, channel: %s", pdu->transm.trans->msg.c_str(), clientChannel->toString().c_str())
		XmsgApMsg::exceptionEnd(clientChannel, pdu, client, RET_EXCEPTION, "x-msg-ap can not found network element to route this message");
		return XscMsgItcpRetType::FORBIDDEN;
	}
	XmsgApMsg::pubMsgRoute2ne(nu, clientChannel, client, pdu); 
	return XscMsgItcpRetType::SUCCESS;
}

XscMsgItcpRetType XmsgApMsg::pubMsgRoute4end(shared_ptr<XscChannel> clientChannel, shared_ptr<XmsgClient> client, shared_ptr<XscProtoPdu> pdu)
{
	if (client == nullptr || !client->isAttached())
	{
		LOG_DEBUG("no permission, message: %s, channel: %s", pdu->transm.trans->msg.c_str(), clientChannel->toString().c_str())
		return XscMsgItcpRetType::FORBIDDEN;
	}
	auto cache = client->delInitTrans(pdu->transm.trans->dtid);
	if (cache == nullptr)
	{
		LOG_DEBUG("can not found x-msg-ne init transaction cache for tid: %08X", pdu->transm.trans->dtid)
		return XscMsgItcpRetType::FORBIDDEN;
	}
	pdu->transm.trans->dtid = cache->tid; 
	shared_ptr<XmsgNeUsrAp> nu = XmsgNeUsrMgrAp::instance()->get(cache->neg);
	if (nu == nullptr) 
	{
		LOG_ERROR("can not found any network element to handle this message, may be all was lost, message: %s, channel: %s", pdu->transm.trans->msg.c_str(), clientChannel->toString().c_str())
		return XscMsgItcpRetType::FORBIDDEN;
	}
	XmsgApMsg::pubMsgRoute2ne(nu, clientChannel, client, pdu); 
	return XscMsgItcpRetType::SUCCESS;
}

XscMsgItcpRetType XmsgApMsg::pubMsgRoute4unidirection(shared_ptr<XscChannel> clientChannel, shared_ptr<XmsgClient> client, shared_ptr<XscProtoPdu> pdu)
{
	if (client == nullptr || !client->isAttached())
	{
		LOG_DEBUG("no permission, message: %s, channel: %s", pdu->transm.trans->msg.c_str(), clientChannel->toString().c_str())
		return XscMsgItcpRetType::FORBIDDEN;
	}
	shared_ptr<XmsgNeUsrAp> nu = XmsgNeUsrMgrAp::instance()->findByMsgName(pdu->transm.trans->msg);
	if (nu == nullptr) 
	{
		LOG_WARN("can not found any network element to handle this message, may be all was lost, message: %s, channel: %s", pdu->transm.trans->msg.c_str(), clientChannel->toString().c_str())
		return XscMsgItcpRetType::FORBIDDEN;
	}
	XmsgApMsg::pubMsgRoute2ne(nu, clientChannel, client, pdu); 
	return XscMsgItcpRetType::SUCCESS;
}

void XmsgApMsg::pubMsgRoute2ne(shared_ptr<XmsgNeUsrAp> ne, shared_ptr<XscChannel> clientChannel, shared_ptr<XmsgClient> client, shared_ptr<XscProtoPdu> pdu)
{
	pdu->transm.addOob(XSC_TAG_UID, client->uid); 
	if (ne->neg != X_MSG_IM_HLR) 
	{
		pdu->transm.addOob(XSC_TAG_CGT, client->cgt); 
		pdu->transm.addOob(XSC_TAG_PLATFORM, client->plat); 
		pdu->transm.addOob(XSC_TAG_DEVICE_ID, client->did);
	}
	if (pdu->transm.trans->refDat && clientChannel->wk != ne->wk) 
		pdu->transm.trans->cloneDat();
	ne->future([ne, clientChannel, client, pdu]
	{
		if (ne->channel->est)
		{
			ne->forward(clientChannel, client, pdu);
			return;
		}
		LOG_WARN("network element channel lost, ne-group: %s, ne: %s", ne->neg.c_str(), ne->uid.c_str()) 
		if (pdu->transm.trans->trans != XSC_TAG_TRANS_BEGIN) 
		{
			return;
		}
		clientChannel->future([ne, clientChannel, client, pdu ]
				{
					XmsgApMsg::exceptionEnd(clientChannel, pdu, client, RET_EXCEPTION, "network element channel lost"); 
				});
	});
}

void XmsgApMsg::exceptionEnd(shared_ptr<XscChannel> clientChannel, shared_ptr<XscProtoPdu> pdu, SptrClient client, ushort ret, const string& desc)
{
	XmsgImChannel::cast(clientChannel)->sendEnd(pdu->transm.trans->stid, ret, desc, nullptr, pdu, [pdu, clientChannel, client](shared_ptr<XscProtoPdu> opdu )
	{
		opdu->takeoffHeader(); 
	});
}

XscMsgItcpRetType XmsgApMsg::priMsgRoute(XscWorker* wk, XscChannel* channel , shared_ptr<XscProtoPdu> pdu)
{
	if (pdu->transm.haveOob(XSC_TAG_INTERCEPT))
		return XscMsgItcpRetType::DISABLE;
	auto usr = channel->usr.lock();
	if (usr == nullptr) 
	{
		LOG_ERROR("remote network element must be pass auth, channel: %s", channel->toString().c_str())
		return XscMsgItcpRetType::EXCEPTION;
	}
	static_pointer_cast<XmsgNeUsrAp>(usr)->receive(pdu);
	return XscMsgItcpRetType::SUCCESS;
}

XmsgApMsg::~XmsgApMsg()
{

}

