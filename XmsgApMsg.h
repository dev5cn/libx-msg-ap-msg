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

#ifndef XMSGAPMSG_H_
#define XMSGAPMSG_H_

#include <libx-msg-ap-core.h>

class XmsgApMsg
{
public:
	static void init(vector<shared_ptr<XmsgImN2HMsgMgr>> pubMsgMgrs, shared_ptr<XmsgImN2HMsgMgr> priMsgMgr); 
private:
	static XscMsgItcpRetType priMsgRoute(XscWorker* wk, XscChannel* channel, shared_ptr<XscProtoPdu> pdu); 
	XmsgApMsg();
	virtual ~XmsgApMsg();
private:
	static XscMsgItcpRetType pubMsgRoute(shared_ptr<XscChannel> clientChannel , SptrClient client , shared_ptr<XscProtoPdu> pdu); 
	static XscMsgItcpRetType pubMsgRoute4begin(shared_ptr<XscChannel> clientChannel, shared_ptr<XmsgClient> client, shared_ptr<XscProtoPdu> pdu); 
	static XscMsgItcpRetType pubMsgRoute4end(shared_ptr<XscChannel> clientChannel, shared_ptr<XmsgClient> client, shared_ptr<XscProtoPdu> pdu); 
	static XscMsgItcpRetType pubMsgRoute4unidirection(shared_ptr<XscChannel> clientChannel, shared_ptr<XmsgClient> client, shared_ptr<XscProtoPdu> pdu); 
	static void pubMsgRoute2ne(shared_ptr<XmsgNeUsrAp> nu, shared_ptr<XscChannel> clientChannel, shared_ptr<XmsgClient> client, shared_ptr<XscProtoPdu> pdu); 
	static void exceptionEnd(shared_ptr<XscChannel> clientChannel, shared_ptr<XscProtoPdu> pdu, SptrClient client, ushort ret, const string& desc); 
};

#endif 
