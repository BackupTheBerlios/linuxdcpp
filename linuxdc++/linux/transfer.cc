/* 
* Copyright (C) 2004 Jens Oknelid, paskharen@spray.se
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "transfer.hh"
#include "guiproxy.hh"
#include <assert.h>

using namespace Gtk;
using namespace SigC;
using namespace SigCX;
using namespace Glib;

CTransfer *CTransfer::instance;
int CTransfer::columnSize[] = { 150, 100, 250, 75, 75, 175, 100, 200, 50, 75 };

ScrolledWindow &CTransfer::getTransferScroll ()
{
	return transferScroll;
}

CTransfer::~CTransfer ()
{
	
}

CTransfer::CTransfer () : 	menuRemoveTransfer("Close transfer"),
										menuBrowse("Browse user filelist"),
										menuRemoveQueue("Remove from queue"),
										menuForce("Force attempt")
{
	Slot1<void, GdkEventButton*> callback1;
	Slot0<void> callback0;
	GuiProxy *proxy = GuiProxy::getInstance();
	ThreadTunnel *tunnel = proxy->getTunnel();

	transferStore = ListStore::create(columns);
	transferList.set_model(transferStore);
	transferList.append_column("User", columns.user);
	transferList.append_column("Hub", columns.hub);
	transferList.append_column("Status", columns.status);
	transferList.append_column("Timeleft", columns.timeleft);
	transferList.append_column("Speed", columns.speed);
	transferList.append_column("File", columns.file);
	transferList.append_column("Size", columns.filesize);
	transferList.append_column("Path", columns.path);
	transferList.append_column("IP", columns.ip);
	transferList.append_column("Ratio", columns.ratio);

	for (int i=0;i<columns.size ()-1;i++)
	{
		transferList.get_column (i)->set_sizing (TREE_VIEW_COLUMN_FIXED);
		transferList.get_column (i)->set_resizable (true);
		transferList.get_column (i)->set_fixed_width (columnSize[i]);
	}

	transferScroll.add (transferList);
	
	proxy->addListener<CTransfer, DownloadManagerListener>(this, DownloadManager::getInstance());
	proxy->addListener<CTransfer, UploadManagerListener>(this, UploadManager::getInstance());
	proxy->addListener<CTransfer, ConnectionManagerListener>(this, ConnectionManager::getInstance());

	callback1 = open_tunnel(tunnel, slot(*this, &CTransfer::showPopupMenu), true);
	transferList.signal_button_press_event().connect_notify(callback1);

	callback0 = open_tunnel(tunnel, slot(*this, &CTransfer::removeTransferClicked), true);
	menuRemoveTransfer.signal_activate().connect(callback0);
	callback0 = open_tunnel(tunnel, slot(*this, &CTransfer::browseClicked), true);
	menuBrowse.signal_activate().connect(callback0);
	callback0 = open_tunnel(tunnel, slot(*this, &CTransfer::removeQueueClicked), true);
	menuRemoveQueue.signal_activate().connect(callback0);

	callback0 = open_tunnel(tunnel, slot(*this, &CTransfer::forceClicked), true);
	menuForce.signal_activate().connect(callback0);

	popupMenu.append (menuForce);
	popupMenu.append (menuSeparator);
	popupMenu.append (menuRemoveTransfer);
	popupMenu.append (menuBrowse);
	popupMenu.append (menuRemoveQueue);
}

TreeModel::iterator CTransfer::findTransfer (TransferItem *t)
{
	TreeModel::iterator it;
	TreeModel::Children c = transferStore->children ();
	
	for (it = c.begin ();it!=c.end();it++)
		if ((*it)[columns.user] == t->user->getNick () && (*it)[columns.direction] == t->type)
			return it;
		
	return c.end ();
}
CTransfer::TransferItem *CTransfer::findTransfer (TreeModel::iterator i)
{
	TransferItem::MapIter it;

	for (it = transfer.begin (); it != transfer.end(); it++)
		if (it->second->user->getNick () == (*i)[columns.user] && it->second->type == (*i)[columns.direction])
			return it->second;

	return NULL;
}

// This function is useless...
CTransfer::TransferItem * CTransfer::bruteFindTransfer ()
{
	TreeModel::iterator it;
	TransferItem::MapIter it2;

	for (it = transferStore->children ().begin (); it != transferStore->children().end(); it++)
		for (it2 = transfer.begin (); it2 != transfer.end (); it2++)
			if (it2->second->user->getNick () == (*it)[columns.user] && it2->second->type == (*it)[columns.direction])
				return it2->second;

	return NULL;
}
// End useless function.




void CTransfer::on(ConnectionManagerListener::Added, ConnectionQueueItem *item) throw()
{
	TransferItem::Types t = item->getConnection () && item->getConnection ()->isSet (UserConnection::FLAG_UPLOAD) ? TransferItem::TYPE_UPLOAD : TransferItem::TYPE_DOWNLOAD;
	TransferItem *i = new TransferItem (item->getUser (), t, TransferItem::STATUS_WAITING);
	Lock l(cs);
	TreeModel::iterator it = findTransfer (i);

	assert (transfer.find (item) == transfer.end());
	transfer.insert (make_pair (item, i));
	if (it == transferStore->children().end())
		it = transferStore->append();

	(*it)[columns.user] = WUtil::ConvertToUTF8 (item->getUser ()->getNick ());
	(*it)[columns.status] = i->statusString = "Connecting...";
	(*it)[columns.direction] = t;
	i->update();
}

void CTransfer::on(ConnectionManagerListener::StatusChanged, ConnectionQueueItem *item) throw()
{
	TransferItem *i;
	Lock l(cs);

	if (transfer.find(item) == transfer.end())
	{
		i = bruteFindTransfer ();
		if (i == NULL)
		{
			cout << "Transfer doesn't exist." << endl;
			return;
		}
	}
	else
		i = transfer[item];
	i->statusString = item->getState () == ConnectionQueueItem::CONNECTING ? "Connecting..." : "Waiting to retry...";

	i->update();
}

void CTransfer::on(ConnectionManagerListener::Removed, ConnectionQueueItem *item) throw()
{
	TransferItem *i;
	Lock l(cs);
	
	if (transfer.find (item) == transfer.end())
	{
		i = bruteFindTransfer ();
		if (!i)
		{
			cout << "Transfer doesn't exist." << endl;
			return;
		}
	}
	else
		i = transfer[item];

	TreeModel::iterator it = findTransfer (i);
	transferStore->erase (it);
	
	if (transfer.find (item) != transfer.end())
		transfer.erase (transfer.find (item));
}

void CTransfer::on(ConnectionManagerListener::Failed, ConnectionQueueItem *item, const string &reason) throw()
{
	TransferItem *i;
	Lock l(cs);

	if (transfer.find(item) == transfer.end())
	{
		i = bruteFindTransfer ();
		if (i == NULL)
		{
			cout << "Transfer doesn't exist." << endl;
			return;
		}
	}
	else
		i = transfer[item];
	i->statusString = reason;

	i->update();
}

void CTransfer::on(DownloadManagerListener::Starting, Download *dl) throw()
{
	ConnectionQueueItem* cqi;
	if (dl->getUserConnection ())
		if (dl->getUserConnection ()->getCQI ())
	 		cqi = dl->getUserConnection()->getCQI();
		
	TransferItem* i;
	Lock l(cs);
	if (dl->getUserConnection ())
	{
		if (dl->getUserConnection ()->getCQI ())
		{
			if (transfer.find(cqi) == transfer.end())
			{
				i = bruteFindTransfer ();
				if (i == NULL)
				{
					cout << "Transfer doesn't exist." << endl;
					return;
				}
			}
			else
			{
				i = transfer[cqi];
			}
		}
	}
	else
	{
		i = bruteFindTransfer ();
		if (i == NULL)
		{
			cout << "Transfer doesn't exist." << endl;
			return;
		}
	}
	i->status = TransferItem::STATUS_RUNNING;
	i->pos = 0;
	i->start = dl->getPos();
	i->actual = i->start;
	i->size = dl->getSize();
	i->file = Util::getFileName(dl->getTarget());
	i->path = Util::getFilePath(dl->getTarget());
	i->statusString = "Download starting...";
	i->IP = dl->getUserConnection()->getRemoteIp();
	
	i->update();
}

void CTransfer::on(DownloadManagerListener::Tick, const Download::List &list) throw()
{
	Lock l(cs);

	char *buf = new char[128];
	
	for(Download::List::const_iterator j = list.begin(); j != list.end(); ++j)
	{
		Download* d = *j;
		sprintf (buf, "Downloaded %s (%.01f%%) in %s" , 	WUtil::ConvertToUTF8 (Util::formatBytes (d->getPos ())).c_str(),
				 																	(double)(d->getPos ()*100.0)/(double)(d->getSize()),
																				WUtil::ConvertToUTF8 (Util::formatSeconds ((GET_TICK() - d->getStart ())/1000)).c_str ());
		ConnectionQueueItem *cqi = d->getUserConnection ()->getCQI ();
		TransferItem *i = transfer[cqi];
		i->actual = i->start + d->getActual();
		i->pos = i->start + d->getTotal();
		i->timeLeft = d->getSecondsLeft();
		i->speed = d->getRunningAverage();

		if(d->isSet(Download::FLAG_ZDOWNLOAD)) {
			i->statusString = "* " + string(buf);
		} else {
			i->statusString = buf;
		}

		i->update();
	}

	delete [] buf;
}

void CTransfer::on(DownloadManagerListener::Failed, Download* dl, const string& reason)throw()
{
	ConnectionQueueItem* cqi;
	if (dl->getUserConnection ())
		if (dl->getUserConnection ()->getCQI ())
	 		cqi = dl->getUserConnection()->getCQI();
		
	TransferItem* i;
	Lock l(cs);
	if (dl->getUserConnection ())
	{
		if (dl->getUserConnection ()->getCQI ())
		{
			if (transfer.find(cqi) == transfer.end())
			{
				i = bruteFindTransfer ();
				if (i == NULL)
				{
					cout << "Transfer doesn't exist." << endl;
					return;
				}
			}
			else
			{
				i = transfer[cqi];
			}
		}
	}
	else
	{
		i = bruteFindTransfer ();
		if (i == NULL)
		{
			cout << "Transfer doesn't exist." << endl;
			return;
		}
	}
	i->status = TransferItem::STATUS_WAITING;
	i->pos = 0;
	i->statusString = reason;
	i->size = dl->getSize();
	i->file = Util::getFileName(dl->getTarget());
	i->path = Util::getFilePath(dl->getTarget());
	
	i->update();
}

void CTransfer::on(UploadManagerListener::Starting, Upload *ul)throw()
{
	ConnectionQueueItem* cqi;
	if (ul->getUserConnection ())
		if (ul->getUserConnection ()->getCQI ())
	 		cqi = ul->getUserConnection()->getCQI();
		
	TransferItem* i;
	Lock l(cs);
	if (ul->getUserConnection ())
	{
		if (ul->getUserConnection ()->getCQI ())
		{
			if (transfer.find(cqi) == transfer.end())
			{
				i = bruteFindTransfer ();
				if (i == NULL)
				{
					cout << "Transfer doesn't exist." << endl;
					return;
				}
			}
			else
			{
				i = transfer[cqi];
			}
		}
	}
	else
	{
		i = bruteFindTransfer ();
		if (i == NULL)
		{
			cout << "Transfer doesn't exist." << endl;
			return;
		}
	}
		
	i->pos = 0;
	i->start = ul->getPos();
	i->actual = i->start;
	i->size = ul->getSize();
	i->status = TransferItem::STATUS_RUNNING;
	i->speed = 0;
	i->timeLeft = 0;

	i->file = Util::getFileName(ul->getFileName());
	i->path = Util::getFilePath(ul->getFileName());
	i->statusString = "Upload starting...";
	i->IP = ul->getUserConnection()->getRemoteIp();

	i->update();
}

void CTransfer::on(UploadManagerListener::Tick, const Upload::List& list) throw()
{
	Lock l(cs);
	char *buf = new char[128];
	for(Upload::List::const_iterator j = list.begin(); j != list.end(); ++j)
	{
		Upload* u = *j;

		ConnectionQueueItem* cqi = u->getUserConnection()->getCQI();
		TransferItem* i = transfer[cqi];
		sprintf (buf, "Uploaded %s (%.01f%%) in %s" , 	WUtil::ConvertToUTF8 (Util::formatBytes (u->getPos ())).c_str(),
				 (double)(u->getPos ()*100.0)/(double)(u->getSize()),
				 WUtil::ConvertToUTF8 (Util::formatSeconds ((GET_TICK() - u->getStart ())/1000)).c_str());

		i->actual = i->start + u->getActual();
		i->pos = i->start + u->getTotal();
		i->timeLeft = u->getSecondsLeft();
		i->speed = u->getRunningAverage();

		if(u->isSet(Upload::FLAG_ZUPLOAD)) {
			i->statusString = "* " + string(buf);
		} else {
			i->statusString = buf;
		}

		i->update();
	}

	delete[] buf;
}

void CTransfer::onTransferComplete(Transfer* t, bool upload)
{
	ConnectionQueueItem* cqi;
	if (t->getUserConnection ())
		if (t->getUserConnection ()->getCQI ())
	 		cqi = t->getUserConnection()->getCQI();

	TransferItem* i;
	Lock l(cs);
	if (t->getUserConnection ())
	{
		if (t->getUserConnection ()->getCQI ())
		{
			if (transfer.find(cqi) == transfer.end())
			{
				i = bruteFindTransfer ();
				if (i == NULL)
				{
					cout << "Transfer doesn't exist." << endl;
					return;
				}
			}
			else
			{
				i = transfer[cqi];
			}
		}
	}
	else
	{
		i = bruteFindTransfer ();
		if (i == NULL)
		{
			cout << "Transfer doesn't exist." << endl;
			return;
		}
	}

	i->status = TransferItem::STATUS_WAITING;
	i->pos = 0;

	i->statusString = upload ? "Upload finished, idle..." : "Download finished, idle...";

	i->update ();
}

void CTransfer::TransferItem::update ()
{
	CTransfer *t = CTransfer::getInstance ();
	TreeModel::iterator it = t->findTransfer (this);
	if (it == t->transferStore->children().end())
	{
		cout << "Couldn't find transfer. Can't update." << endl;
		return;
	}
	
	(*it)[t->columns.user] = WUtil::ConvertToUTF8 (user->getNick ());
	(*it)[t->columns.hub] = WUtil::ConvertToUTF8 (user->getClientName ());
	(*it)[t->columns.status] = WUtil::ConvertToUTF8 (statusString);
	(*it)[t->columns.timeleft] = (status == STATUS_RUNNING) ? WUtil::ConvertToUTF8 (Util::formatSeconds (timeLeft)) : "";
	(*it)[t->columns.speed] = (status == STATUS_RUNNING) ? WUtil::ConvertToUTF8 (Util::formatBytes (speed) + "/s") : "";
	(*it)[t->columns.file] = WUtil::ConvertToUTF8 (file);
	(*it)[t->columns.filesize] = WUtil::ConvertToUTF8 (Util::formatBytes (size));
	(*it)[t->columns.path] = WUtil::ConvertToUTF8 (path);
	(*it)[t->columns.ip] = WUtil::ConvertToUTF8 (IP);
	(*it)[t->columns.ratio] = WUtil::ConvertToUTF8 (Util::toString (getRatio ()));
}

void CTransfer::TransferItem::disconnect ()
{
	ConnectionManager::getInstance()->removeConnection(user, (type == TYPE_DOWNLOAD));
}

void CTransfer::TransferItem::remove()
{
}

CTransfer *CTransfer::getInstance ()
{
	return instance;
}

void CTransfer::startup ()
{
	instance = new CTransfer ();
}

void CTransfer::shutdown ()
{
	GuiProxy::getInstance ()->removeListener<CTransfer> (this);

	delete this;
}

void CTransfer::showPopupMenu(GdkEventButton* event)
{
	if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3))
	{
		popupMenu.popup(event->button, event->time);
		popupMenu.show_all();
	}
}

void CTransfer::removeTransferClicked ()
{
	TreeModel::iterator it = transferList.get_selection ()->get_selected ();
	TransferItem *i;

	if (!it)
		return;

	i = findTransfer (it);

	if (!i)
		return;

	i->disconnect ();
}

void CTransfer::forceClicked ()
{
	TreeModel::iterator it = transferList.get_selection ()->get_selected ();
	TransferItem *i;

	if (!it)
		return;

	i = findTransfer (it);

	if (!i)
		return;

	i->user->connect ();
	(*it)[columns.status] = "Connecting (forced)...";
}

void CTransfer::browseClicked ()
{
	ShareBrowser *b;
TreeModel::iterator it = transferList.get_selection ()->get_selected ();

	ClientManager *man = ClientManager::getInstance();
	
	if (!it) return;
	TransferItem *i = findTransfer (it);
	
	try
	{
		QueueManager::getInstance ()->addList(i->user, QueueItem::FLAG_CLIENT_VIEW);
	}
	catch(...)
	{
		cout << "Couldn't add filelist." << endl;
	}	
}

void CTransfer::removeQueueClicked ()
{

}
