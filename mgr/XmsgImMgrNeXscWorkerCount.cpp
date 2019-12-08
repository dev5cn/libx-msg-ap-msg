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

#include "XmsgImMgrNeXscWorkerCount.h"

XmsgImMgrNeXscWorkerCount::XmsgImMgrNeXscWorkerCount()
{

}

void XmsgImMgrNeXscWorkerCount::handle(shared_ptr<XmsgNeUsr> nu, SptrXitp trans, shared_ptr<XmsgImMgrNeXscWorkerCountReq> req)
{
	if (req->name().empty())
	{
		trans->endDesc(RET_FORMAT_ERROR, "xsc-server name can not be null");
		return;
	}
	auto server = XscServer::get(req->name());
	if (server == nullptr)
	{
		trans->endDesc(RET_FORBIDDEN, "can not found xsc-server for name: %s", req->name().c_str());
		return;
	}
	shared_ptr<XmsgImMgrNeXscWorkerCountRsp> rsp(new XmsgImMgrNeXscWorkerCountRsp());
	rsp->set_count(server->xscWorker.size());
	trans->end(rsp);
}

XmsgImMgrNeXscWorkerCount::~XmsgImMgrNeXscWorkerCount()
{

}

