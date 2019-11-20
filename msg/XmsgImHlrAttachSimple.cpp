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
#include "XmsgImHlrAttachSimple.h"

XmsgImHlrAttachSimple::XmsgImHlrAttachSimple()
{

}

void XmsgImHlrAttachSimple::handle(shared_ptr<XscChannel> channel, SptrXitp trans, shared_ptr<XmsgImHlrAttachSimpleReq> req)
{
	if (req->token().empty())
	{
		trans->endDesc(RET_FORMAT_ERROR, "token format error");
		return;
	}
	if (req->salt().length() < X_MSG_LEN_MIN_SALT)
	{
		trans->endDesc(RET_FORMAT_ERROR, "salt format error, must be great than 8bytes");
		return;
	}
	if (req->sign().empty())
	{
		trans->endDesc(RET_FORMAT_ERROR, "sign format error");
		return;
	}
	if (req->cgt().empty())
	{
		trans->endDesc(RET_FORMAT_ERROR, "channel global title format error");
		return;
	}
	SptrCgt cgt = ChannelGlobalTitle::parse(req->cgt());
	if (cgt == nullptr)
	{
		trans->endDesc(RET_FORMAT_ERROR, "channel global title format error");
		return;
	}
	SptrOob oob(new list<pair<uchar, string>>());
	string ccid = XmsgClient::genCcid();
	oob->push_back(make_pair<>(XSC_TAG_UID, ccid));
	oob->push_back(make_pair<>(XSC_TAG_INTERCEPT, "enable"));
	auto client = SptrClient(new XmsgClient(req->cgt(), "", "", ccid, trans->channel)); 
	trans->channel->setXscUsr(client);
	XmsgClientMgr::instance()->addXmsgClient(client);
	XmsgImHlrAttachSimple::attach4local(trans, req, client, cgt, oob);
}

void XmsgImHlrAttachSimple::attach4local(SptrXitp trans, shared_ptr<XmsgImHlrAttachSimpleReq> req, SptrClient client, SptrCgt cgt, SptrOob oob)
{
	shared_ptr<XmsgNeUsr> nu = XmsgNeUsrMgrAp::instance()->getHlr(); 
	if (nu == nullptr)
	{
		LOG_ERROR("can not allocate x-msg-im-hlr, req: %s", req->ShortDebugString().c_str())
		trans->endAndLazyClose(RET_EXCEPTION, "can not allocate x-msg-im-hlr");
		return;
	}
	XmsgImChannel::cast(nu->channel)->begin(req, [req, trans, client](SptrXiti itrans) 
	{
		XmsgImHlrAttachSimple::handleRsp(trans, req, client, itrans);
	}, oob, trans);
}

void XmsgImHlrAttachSimple::handleRsp(SptrXitp trans, shared_ptr<XmsgImHlrAttachSimpleReq> req, SptrClient client, SptrXiti itrans)
{
	if (itrans->ret != RET_SUCCESS || itrans->endMsg == nullptr) 
	{
		trans->endAndLazyClose(itrans->ret, itrans->desc.c_str());
		return;
	}
	string plat;
	if (!itrans->getOob(XSC_TAG_PLATFORM, plat))
	{
		LOG_FAULT("x-msg-im-hlr must be set oob XSC_TAG_PLATFORM")
		trans->endAndLazyClose(RET_EXCEPTION, "system exception");
		return;
	}
	string did;
	if (!itrans->getOob(XSC_TAG_DEVICE_ID, did))
	{
		LOG_FAULT("x-msg-im-hlr must be set oob XSC_TAG_DEVICE_ID")
		trans->endAndLazyClose(RET_EXCEPTION, "system exception");
		return;
	}
	auto rsp = static_pointer_cast<XmsgImHlrAttachSimpleRsp>(itrans->endMsg);
	client->plat = plat;
	client->did = did;
	LOG_DEBUG("x-msg-client attach successful, cgt: %s, plat: %s, did: %s, ccid: %s, req: %s, rsp: %s", 
			client->cgt.c_str(),
			plat.c_str(),
			did.c_str(),
			client->uid.c_str(),
			trans->beginMsg->ShortDebugString().c_str(),
			rsp->ShortDebugString().c_str())
	trans->end(rsp); 
}

XmsgImHlrAttachSimple::~XmsgImHlrAttachSimple()
{

}

