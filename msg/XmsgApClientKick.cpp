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

#include "XmsgApClientKick.h"

XmsgApClientKick::XmsgApClientKick()
{

}

void XmsgApClientKick::handle(shared_ptr<XmsgNeUsr> nu, SptrXitp trans, shared_ptr<XmsgApClientKickReq> req)
{
	SptrClient client = XmsgClientMgr::instance()->findXmsgClient(req->ccid());
	if (client == nullptr)
	{
		trans->endDesc(RET_NOT_FOUND, "can not found x-msg-client for ccid: %s, may be it was lost", req->ccid().c_str());
		return;
	}
	client->future([client]
	{
		if (!client->channel->est)
		{
			LOG_DEBUG("x-msg-client channel already lost: %s", client->toString().c_str())
			return;
		}
		client->channel->close();
	});
	shared_ptr<XmsgApClientKickRsp> rsp(new XmsgApClientKickRsp());
	XmsgMisc::insertKv(rsp->mutable_ext(), "accept", "true");
	trans->end(rsp);
}

XmsgApClientKick::~XmsgApClientKick()
{

}

