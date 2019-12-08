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

#include "XmsgImMgrNeNetLoad.h"

XmsgImMgrNeNetLoad::XmsgImMgrNeNetLoad()
{

}

void XmsgImMgrNeNetLoad::handle(shared_ptr<XmsgNeUsr> nu, SptrXitp trans, shared_ptr<XmsgImMgrNeNetLoadReq> req)
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
	if (req->indx() == 0xFFFFFFFF)
	{
		XmsgImMgrNeNetLoad::handle4all(nu, trans, req, server);
		return;
	}
	XmsgImMgrNeNetLoad::handle4worker(nu, trans, req, server);
}

void XmsgImMgrNeNetLoad::handle4all(shared_ptr<XmsgNeUsr> nu, SptrXitp trans, shared_ptr<XmsgImMgrNeNetLoadReq> req, shared_ptr<XscServer> server)
{
	shared_ptr<XmsgImMgrNeNetLoadRsp> rsp(new XmsgImMgrNeNetLoadRsp());
	rsp->Clear();
	for (size_t i = 0; i < server->xscWorker.size(); ++i)
	{
		auto stat = server->xscWorker.at(i)->stat;
		rsp->set_rxbytes(rsp->rxbytes() + stat->get(XscWorkerStatItem::XSC_WORKER_RX_BYTES));
		rsp->set_rxmsgs(rsp->rxmsgs() + stat->get(XscWorkerStatItem::XSC_WORKER_RX_MSGS));
		rsp->set_txbytes(rsp->txbytes() + stat->get(XscWorkerStatItem::XSC_WORKER_RX_MSGS));
		rsp->set_txmsgs(rsp->txmsgs() + stat->get(XscWorkerStatItem::XSC_WORKER_TX_MSGS));
		rsp->set_n2htotal(rsp->n2htotal() + stat->get(XscWorkerStatItem::XSC_WORKER_N2H_TOTAL));
		rsp->set_n2hdestory(rsp->n2hdestory() + stat->get(XscWorkerStatItem::XSC_WORKER_N2H_DESTORY));
	}
	trans->end(rsp);
}

void XmsgImMgrNeNetLoad::handle4worker(shared_ptr<XmsgNeUsr> nu, SptrXitp trans, shared_ptr<XmsgImMgrNeNetLoadReq> req, shared_ptr<XscServer> server)
{
	if (req->indx() >= server->xscWorker.size())
	{
		trans->endDesc(RET_FORBIDDEN, "over the xsc-worker index");
		return;
	}
	shared_ptr<XmsgImMgrNeNetLoadRsp> rsp(new XmsgImMgrNeNetLoadRsp());
	auto stat = server->xscWorker.at(req->indx())->stat;
	rsp->set_rxbytes(stat->get(XscWorkerStatItem::XSC_WORKER_RX_BYTES));
	rsp->set_rxmsgs(stat->get(XscWorkerStatItem::XSC_WORKER_RX_MSGS));
	rsp->set_txbytes(stat->get(XscWorkerStatItem::XSC_WORKER_RX_MSGS));
	rsp->set_txmsgs(stat->get(XscWorkerStatItem::XSC_WORKER_TX_MSGS));
	rsp->set_n2htotal(stat->get(XscWorkerStatItem::XSC_WORKER_N2H_TOTAL));
	rsp->set_n2hdestory(stat->get(XscWorkerStatItem::XSC_WORKER_N2H_DESTORY));
	trans->end(rsp);
}

XmsgImMgrNeNetLoad::~XmsgImMgrNeNetLoad()
{

}

