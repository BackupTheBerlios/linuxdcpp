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

#include "guiproxy.hh"
#include "queue.hh"

using namespace std;
using namespace Gtk;
using namespace SigC;
using namespace SigCX;
using namespace Glib;

int DownloadQueue::columnSize[] = { 200, 300, 75, 110, 75, 200, 200, 75, 200, 100, 125 };
DownloadQueue::~DownloadQueue ()
{
	GuiProxy::getInstance()->removeListener<DownloadQueue>(this);
}

DownloadQueue::DownloadQueue (MainWindow *mw) :
	smDirPrio ("Set priority"),
	smFilePrio ("Set priority"),
	smBrowse ("Get file list"),
	smPM ("Send private message"),
	smReAdd ("Re-add source"),
	smRemove ("Remove source"),
	smRemoveAll ("Remove user from queue")
{
	Slot0<void> callback0;
	Slot1<void, GdkEventButton *> callback1;
	GuiProxy *proxy = GuiProxy::getInstance();
	ThreadTunnel *tunnel = proxy->getTunnel();
		
	this->window = mw;
	this->ID = BOOk_DOWNLOAD_QUEUE;

	label.set_text("Download queue");
	label.show();

	add (mainBox);
	
	dirStore = TreeStore::create(dirColumns);
	dirView.set_model(dirStore);
	dirView.append_column("Directory", dirColumns.name);

	fileStore = ListStore::create(fileColumns);
	fileView.set_model(fileStore);
	fileView.append_column("Filename", fileColumns.target);
	fileView.append_column("Status", fileColumns.status);
	fileView.append_column("Size", fileColumns.filesize);
	fileView.append_column("Downloaded", fileColumns.downloaded);
	fileView.append_column("Priority", fileColumns.priority);
	fileView.append_column("Users", fileColumns.users);
	fileView.append_column("Path", fileColumns.path);
	fileView.append_column("Exact size", fileColumns.exactsize);
	fileView.append_column("Error", fileColumns.errors);
	fileView.append_column("Added", fileColumns.added);
	fileView.append_column("TTH", fileColumns.tth);
	

	for (int i=0;i<fileColumns.size ()-1;i++)
	{
		fileView.get_column (i)->set_sizing (TREE_VIEW_COLUMN_FIXED);
		fileView.get_column (i)->set_resizable (true);
		fileView.get_column (i)->set_fixed_width (columnSize[i]);
	}

	dirScroll.set_policy(POLICY_AUTOMATIC, POLICY_AUTOMATIC);
	dirScroll.add(dirView);
	fileScroll.set_policy(POLICY_AUTOMATIC, POLICY_AUTOMATIC);
	fileScroll.add(fileView);
	
	pane.add1(dirScroll);
	pane.add2(fileScroll);
	pane.set_position(200);

	mainBox.pack_start (pane, PACK_EXPAND_WIDGET, 0);
	mainBox.pack_start (statusBox, PACK_SHRINK, 0);

	
	for (int i=0;i<STATUS_LAST;i++)
	{
		statusBar[i].set_has_resize_grip (false);
		if (i==0)
			statusBox.pack_start (statusBar[i], PACK_EXPAND_WIDGET, 2);
		else
			statusBox.pack_start (statusBar[i], PACK_SHRINK, 2);
		
		if (i == 1 || i == 3)	
			statusBar[i].set_size_request (60, -1);
		else
			statusBar[i].set_size_request (100, -1);
	}
	queueItems = 0;
	queueSize = 0;
	updateStatus ();
	buildList (QueueManager::getInstance ()->lockQueue ());
	QueueManager::getInstance()->unlockQueue();

	callback1 = open_tunnel(tunnel, slot(*this, &DownloadQueue::buttonPressedDir), true);
	dirView.signal_button_press_event().connect_notify(callback1);
	callback1 = open_tunnel(tunnel, slot(*this, &DownloadQueue::buttonReleasedDir), true);
	dirView.signal_button_release_event().connect_notify(callback1);

	callback1 = open_tunnel(tunnel, slot(*this, &DownloadQueue::buttonPressedFile), true);
	fileView.signal_button_press_event().connect_notify(callback1);
	callback1 = open_tunnel(tunnel, slot(*this, &DownloadQueue::buttonReleasedFile), true);
	fileView.signal_button_release_event().connect_notify(callback1);

	proxy->addListener<DownloadQueue, QueueManagerListener>(this, QueueManager::getInstance());

	smDirPrio.set_submenu (priorityDirMenu);
	smFilePrio.set_submenu (priorityFileMenu);
	smBrowse.set_submenu (browseMenu);
	smPM.set_submenu (pmMenu);
	smReAdd.set_submenu (readdMenu),
	smRemove.set_submenu (removeMenu),
	smRemoveAll.set_submenu (removeAllMenu);
	
	using namespace Gtk::Menu_Helpers;
	MenuList items = priorityDirMenu.items();
	items.push_back (MenuElem ("Paused"));
	items.push_back (MenuElem ("Lowest"));
	items.push_back (MenuElem ("Low"));
	items.push_back (MenuElem ("Normal"));
	items.push_back (MenuElem ("High"));
	items.push_back (MenuElem ("Highest"));

	items = priorityFileMenu.items();
	items.push_back (MenuElem ("Paused"));
	items.push_back (MenuElem ("Lowest"));
	items.push_back (MenuElem ("Low"));
	items.push_back (MenuElem ("Normal"));
	items.push_back (MenuElem ("High"));
	items.push_back (MenuElem ("Highest"));	

	items = dirMenu.items();
	items.push_back (smDirPrio);
	items.push_back (MenuElem ("Move/Rename"));
	items.push_back (SeparatorElem ());
	items.push_back (MenuElem ("Remove", open_tunnel (tunnel, slot(*this, &DownloadQueue::removeDirClicked), true)));

	items = fileMenu.items();
	items.push_back (MenuElem ("Search for alternates", open_tunnel(tunnel, slot(*this, &DownloadQueue::searchAlternates), true)));
	items.push_back (MenuElem ("Search by TTH"));
	items.back ().set_sensitive (false);
	items.push_back (MenuElem ("Move/Rename"));
	items.back ().set_sensitive (false);
	items.push_back (SeparatorElem ());
	items.push_back (smFilePrio);
	items.push_back (smBrowse);
	items.push_back (smPM);
	items.push_back (smReAdd);
	items.push_back (SeparatorElem ());
	items.push_back (smRemove);
	items.push_back (smRemoveAll);
	items.push_back (MenuElem ("Remove", open_tunnel (tunnel, slot(*this, &DownloadQueue::removeFileClicked), true)));
}

void DownloadQueue::setStatus (std::string text, int num)
{
	if (num<0 || num>STATUS_LAST-1) return;

	statusBar[num].pop(1);
	if (num == 0)
		statusBar[num].push ("[" + Util::getShortTimeString() + "] " + text, 1);
	else
		statusBar[num].push (text, 1);
}
void DownloadQueue::searchAlternates ()
{
	TreeModel::Row r = *(fileView.get_selection()->get_selected());

	if (!r)
		return;

	QueueItemInfo *i = r[fileColumns.item];
	
	ustring searchString =  r[fileColumns.target];

	if (!searchString.empty ())
	{
		Search *s = new Search (window);
		if (window->addPage (s))
			manage (s);
		else
			delete s;
		if (i->getSize () > 10*1024*1024)
			s->searchFor (searchString);
		else
			s->searchFor (searchString);
	}
}

void DownloadQueue::searchTTH ()
{

}

void DownloadQueue::browseList (QueueItemInfo::SourceInfo *s)
{
	try
	{
		QueueManager::getInstance ()->addList(s->getUser (), QueueItem::FLAG_CLIENT_VIEW);
	}
	catch(...)
	{
		cout << "Couldn't add filelist." << endl;
	}
}

void DownloadQueue::sendPM (QueueItemInfo::SourceInfo *s)
{
	PrivateMsg *privMsg;
	
	privMsg = new PrivateMsg(s->getUser (), window);
	window->addPage(privMsg);
}
void DownloadQueue::readdSource (QueueItemInfo *i, QueueItemInfo::SourceInfo *s)
{
	try
	{
		QueueManager::getInstance()->readd(i->getTarget(), s->getUser());
	}
	catch(...)
	{
		cout << "Couldn't readd: " << endl;
	}	
}
void DownloadQueue::removeSource (QueueItemInfo *i, QueueItemInfo::SourceInfo *s)
{
	QueueManager::getInstance()->removeSource(i->getTarget(), s->getUser(), QueueItem::Source::FLAG_REMOVED);
}

void DownloadQueue::removeUser (QueueItemInfo *i, QueueItemInfo::SourceInfo *s)
{
	QueueManager::getInstance()->removeSources(s->getUser(), QueueItem::Source::FLAG_REMOVED);
}

bool DownloadQueue::operator== (BookEntry &b) {
	DownloadQueue *queue;
	
	queue = dynamic_cast<DownloadQueue *>(&b);
	return queue != NULL;
}

string DownloadQueue::getNextSubDir (string path)
{
	string tmp = path.substr (1, path.find_first_of ('/', 1));
	if (tmp != "")
	{
		tmp.erase (tmp.size ()-1, 1);
		return tmp;
	}
	return tmp;
}
string DownloadQueue::getRemainingDir (string path)
{
	string tmp = path.substr (path.find_first_of ('/', 1), path.size ()-path.find_first_of ('/', 1));
	return tmp;
}
void DownloadQueue::rebuildMenu ()
{
	Slot0<void> callback;
	GuiProxy *proxy = GuiProxy::getInstance();
	ThreadTunnel *tunnel = proxy->getTunnel();
	TreeModel::Row r = *(fileView.get_selection()->get_selected());

	if (!r)
		return;

	QueueItemInfo *i = r[fileColumns.item];
	using namespace Gtk::Menu_Helpers;
    MenuList browse = browseMenu.items(), pm = pmMenu.items(), readd = readdMenu.items(), remove = removeMenu.items(), removeAll = removeAllMenu.items ();
	browse.clear ();
	pm.clear ();
	readd.clear();
	remove.clear();
	removeAll.clear();
	for (QueueItemInfo::SourceIter it=i->getSources ().begin (); it != i->getSources ().end (); it++)
	{
		//if (it->getUser ()->isOnline ())
		{
			browse.push_back (MenuElem (it->getUser()->getFullNick(), open_tunnel(tunnel, bind<QueueItemInfo::SourceInfo*>(slot (*this, &DownloadQueue::browseList), &(*it)), true)));
			pm.push_back (MenuElem (it->getUser()->getFullNick()));
			pm.back ().signal_activate ().connect (open_tunnel(tunnel, bind<QueueItemInfo::SourceInfo*>(slot (*this, &DownloadQueue::sendPM), &(*it)), true));
			readd.push_back (MenuElem (it->getUser()->getFullNick()));
			readd.back ().signal_activate ().connect (open_tunnel(tunnel, bind<QueueItemInfo*, QueueItemInfo::SourceInfo*>(slot (*this, &DownloadQueue::readdSource), i, &(*it)), true));
			remove.push_back (MenuElem (it->getUser()->getFullNick()));
			remove.back ().signal_activate ().connect (open_tunnel(tunnel, bind<QueueItemInfo*, QueueItemInfo::SourceInfo*>(slot (*this, &DownloadQueue::removeSource), i, &(*it)), true));
			removeAll.push_back (MenuElem(it->getUser()->getFullNick()));
			removeAll.back ().signal_activate ().connect (open_tunnel(tunnel, bind<QueueItemInfo*, QueueItemInfo::SourceInfo*>(slot (*this, &DownloadQueue::removeUser), i, &(*it)), true));
		}	
	}
}
void DownloadQueue::QueueItemInfo::update (DownloadQueue *dq, bool add)
{
	TreeModel::iterator item;
	if (!add)
	{
		bool found=false;
		for (item = dq->fileStore->children ().begin (); item != dq->fileStore->children ().end (); item++)
			if (((QueueItemInfo*)(*item)[dq->fileColumns.item]) == this)
			{
				found = true;
				break;
			}

		if (!found)
			return;
		// Users
		int online=0;
		{
			ustring tmp;
			QueueItemInfo::SourceIter j;
			for(j = getSources().begin(); j != getSources().end(); ++j)
			{
				if(tmp.size() > 0)
					tmp += ", ";
	
					if(j->getUser()->isOnline())
						online++;
	
					tmp += j->getUser()->getFullNick();
			}
			(*item)[dq->fileColumns.users] = tmp.empty() ? "No users" : tmp;
		}		
		// Status
		{
			if(getStatus() == QueueItem::STATUS_WAITING)
			{
				char buf[64];
				if(online > 0)
				{
					if(getSources().size() == 1)
						(*item)[dq->fileColumns.status] = "Waiting (User online)";
					else
					{
						sprintf(buf, "Waiting (%d of %d users online), online", getSources().size());
						(*item)[dq->fileColumns.status] = buf;
					}
				}
				else
				{
					if(getSources().size() == 0)
						(*item)[dq->fileColumns.status] = "No users to download from";
					else if(getSources().size() == 1)
						(*item)[dq->fileColumns.status] = "User offline";
					else if(getSources().size() == 2)
						(*item)[dq->fileColumns.status] = "Both users offline";
					else
					{
						sprintf(buf, "All %d users offline", getSources().size());
						(*item)[dq->fileColumns.status] = buf;
					}
				}
			}
			else if(getStatus() == QueueItem::STATUS_RUNNING)
				(*item)[dq->fileColumns.status] = "Running...";
		}
		// Downloaded
		{
			if(getSize() > 0)
				(*item)[dq->fileColumns.downloaded] = Util::formatBytes(getDownloadedBytes()) + " (" + Util::toString((double)getDownloadedBytes()*100.0/(double)getSize()) + "%)";
			else
				(*item)[dq->fileColumns.downloaded] = "";;
		}
		// Priority
		{
			switch(getPriority())
			{
				case QueueItem::PAUSED: (*item)[dq->fileColumns.priority] = "Paused"; break;
				case QueueItem::LOWEST: (*item)[dq->fileColumns.priority] = "Lowest"; break;
				case QueueItem::LOW: (*item)[dq->fileColumns.priority] = "Low"; break;
				case QueueItem::NORMAL: (*item)[dq->fileColumns.priority] = "Normal"; break;
				case QueueItem::HIGH: (*item)[dq->fileColumns.priority] = "High"; break;
				case QueueItem::HIGHEST: (*item)[dq->fileColumns.priority] = "Highest"; break;
			}
		}
		// Errors
		{
			ustring tmp;
			QueueItemInfo::SourceIter j;
			for(j = getBadSources().begin(); j != getBadSources().end(); ++j)
			{
				if(!j->isSet(QueueItem::Source::FLAG_REMOVED))
				{
					if(tmp.size() > 0)
						tmp += ", ";
					tmp += j->getUser()->getNick();
					tmp += " (";
					if(j->isSet(QueueItem::Source::FLAG_FILE_NOT_AVAILABLE))
						tmp += "File not available";
					else if(j->isSet(QueueItem::Source::FLAG_PASSIVE))
						tmp += "Passive user";
					else if(j->isSet(QueueItem::Source::FLAG_ROLLBACK_INCONSISTENCY))
						tmp += "Rollback inconsistency, existing file does not match the one being downloaded";
					else if(j->isSet(QueueItem::Source::FLAG_CRC_FAILED))
						tmp += "CRC32 inconsistency (SFV-Check)";
					else if(j->isSet(QueueItem::Source::FLAG_BAD_TREE))
						tmp += "Downloaded tree does not match TTH root";
					tmp += ")";
				}
			}
			(*item)[dq->fileColumns.errors] = tmp.empty() ? "No errors" : tmp;
		}		
		// TTH
		{
			if (getTTH () != NULL)
				(*item)[dq->fileColumns.tth] = getTTH()->toBase32();
		}				
	}
	else
	{
		TreeModel::Row row = *dq->fileStore->append ();
		// Item
		row[dq->fileColumns.item] = this;
		// Filename
		row[dq->fileColumns.target] = 
			WUtil::ConvertToUTF8(Util::getFileName(getTarget()));
		// Users
		int online=0;
		{
			ustring tmp;
			QueueItemInfo::SourceIter j;
			for(j = getSources().begin(); j != getSources().end(); ++j)
			{
				if(tmp.size() > 0)
					tmp += ", ";
	
					if(j->getUser()->isOnline())
						online++;
	
					tmp += j->getUser()->getFullNick();
			}
			row[dq->fileColumns.users] = tmp.empty() ? "No users" : tmp;
		}		
		// Status
		{
			if(getStatus() == QueueItem::STATUS_WAITING)
			{
				char buf[64];
				if(online > 0)
				{
					if(getSources().size() == 1)
						row[dq->fileColumns.status] = "Waiting (User online)";
					else
					{
						sprintf(buf, "Waiting (%d of %d users online), online", getSources().size());
						row[dq->fileColumns.status] = buf;
					}
				}
				else
				{
					if(getSources().size() == 0)
						row[dq->fileColumns.status] = "No users to download from";
					else if(getSources().size() == 1)
						row[dq->fileColumns.status] = "User offline";
					else if(getSources().size() == 2)
						row[dq->fileColumns.status] = "Both users offline";
					else
					{
						sprintf(buf, "All %d users offline", getSources().size());
						row[dq->fileColumns.status] = buf;
					}
				}
			}
			else if(getStatus() == QueueItem::STATUS_RUNNING)
				row[dq->fileColumns.status] = "Running...";
		}
		// Size
		{
			row[dq->fileColumns.filesize] = (getSize() == -1) ? "Unkown" : Util::formatBytes(getSize());
			row[dq->fileColumns.exactsize] = (getSize() == -1) ? "Unkown" : Util::formatExactSize(getSize());
		}
		// Downloaded
		{
			if(getSize() > 0)
				row[dq->fileColumns.downloaded] = Util::formatBytes(getDownloadedBytes()) + " (" + Util::toString((double)getDownloadedBytes()*100.0/(double)getSize()) + "%)";
			else
				row[dq->fileColumns.downloaded] = "";;
		}
		// Priority
		{
			switch(getPriority())
			{
				case QueueItem::PAUSED: row[dq->fileColumns.priority] = "Paused"; break;
				case QueueItem::LOWEST: row[dq->fileColumns.priority] = "Lowest"; break;
				case QueueItem::LOW: row[dq->fileColumns.priority] = "Low"; break;
				case QueueItem::NORMAL: row[dq->fileColumns.priority] = "Normal"; break;
				case QueueItem::HIGH: row[dq->fileColumns.priority] = "High"; break;
				case QueueItem::HIGHEST: row[dq->fileColumns.priority] = "Highest"; break;
			}
		}
		// Path
		{
			row[dq->fileColumns.path] = 
				WUtil::ConvertToUTF8(Util::getFilePath(getTarget()));
		}
		// Errors
		{
			ustring tmp;
			QueueItemInfo::SourceIter j;
			for(j = getBadSources().begin(); j != getBadSources().end(); ++j)
			{
				if(!j->isSet(QueueItem::Source::FLAG_REMOVED))
				{
					if(tmp.size() > 0)
						tmp += ", ";
					tmp += j->getUser()->getNick();
					tmp += " (";
					if(j->isSet(QueueItem::Source::FLAG_FILE_NOT_AVAILABLE))
						tmp += "File not available";
					else if(j->isSet(QueueItem::Source::FLAG_PASSIVE))
						tmp += "Passive user";
					else if(j->isSet(QueueItem::Source::FLAG_ROLLBACK_INCONSISTENCY))
						tmp += "Rollback inconsistency, existing file does not match the one being downloaded";
					else if(j->isSet(QueueItem::Source::FLAG_CRC_FAILED))
						tmp += "CRC32 inconsistency (SFV-Check)";
					else if(j->isSet(QueueItem::Source::FLAG_BAD_TREE))
						tmp += "Downloaded tree does not match TTH root";
					tmp += ")";
				}
			}
			row[dq->fileColumns.errors] = tmp.empty() ? "No errors" : tmp;
		}
		// Added
		{
			row[dq->fileColumns.added] = Util::formatTime("%Y-%m-%d %H:%M", getAdded());
		}
		// TTH
		{
			if (getTTH () != NULL)
				row[dq->fileColumns.tth] = getTTH()->toBase32();
		}		
	}
}
void DownloadQueue::updateStatus ()
{
	TreeModel::Row r = *(dirView.get_selection()->get_selected());

	if (!r)
	{
		setStatus ("Items: 0", STATUS_ITEMS);
		setStatus ("Size: 0 B", STATUS_FILE_SIZE);
		setStatus ("File: " + Util::toString (queueItems), STATUS_FILES);
		setStatus ("Size: " + Util::formatBytes (queueSize), STATUS_TOTAL_SIZE);
		return;
	}
		
	int64_t total=0;
	int count=0;

	/* TODO: Add selection. */
	{
		for (vector<QueueItemInfo*>::iterator it=dirFileMap[r[dirColumns.realpath]].begin (); it != dirFileMap[r[dirColumns.realpath]].end (); it++)
		{
			count++;
			total += ((*it)->getSize() > 0) ? (*it)->getSize() : 0;
		}
	}

	setStatus ("Items: " + Util::toString (count), STATUS_ITEMS);
	setStatus ("Size: " + Util::formatBytes (total), STATUS_FILE_SIZE);
	setStatus ("File: " + Util::toString (queueItems), STATUS_FILES);
	setStatus ("Size: " + Util::formatBytes (queueSize), STATUS_TOTAL_SIZE);
}
void DownloadQueue::update ()
{
	TreeModel::Row r = *(dirView.get_selection()->get_selected());

	if (!r)
		return;

	fileStore->clear ();

	TreeModel::Row row;
	showingDir = r[dirColumns.realpath];
	for (vector<QueueItemInfo*>::iterator it=dirFileMap[r[dirColumns.realpath]].begin (); it != dirFileMap[r[dirColumns.realpath]].end (); it++)
	{
		(*it)->update (this, true);
	}
	updateStatus ();
}

void DownloadQueue::buildList (const QueueItem::StringMap &l)
{
	TreeModel::Row row;
	queueItems = 0;
	queueSize = 0;
	for (QueueItem::StringMap::const_iterator it = l.begin (); it != l.end (); it++)
	{
		if (dirMap.find ("/" + getNextSubDir (Util::getFilePath(it->second->getTarget())) + "/") == dirMap.end ())
		{
			row = *(dirStore->append());
			row[dirColumns.name] = 
				WUtil::ConvertToUTF8(getNextSubDir (Util::getFilePath(it->second->getTarget())));
			row[dirColumns.realpath] = 
				WUtil::ConvertToUTF8("/" + getNextSubDir (Util::getFilePath(it->second->getTarget())) + "/");
			dirMap[row[dirColumns.realpath]] = row;
			ustring tmp;;
			addDir (getRemainingDir (Util::getFilePath(it->second->getTarget())), row, tmp);
			fileMap[it->second->getTarget ()] = it->second;
			addFile (new QueueItemInfo (it->second), tmp);
		}
		else
		{
			ustring tmp;
			addDir (getRemainingDir (Util::getFilePath(it->second->getTarget())),
						 dirMap["/" + getNextSubDir (Util::getFilePath(it->second->getTarget())) + "/"], tmp);
			fileMap[it->second->getTarget ()] = it->second;
			addFile (new QueueItemInfo (it->second), tmp);
		}
	}
}
void DownloadQueue::addDir (string path, TreeModel::Row row, ustring &current)
{
	TreeModel::Row newRow;
	string tmp = getNextSubDir (path);
	
	if (tmp == "")
	{
		current = row[dirColumns.realpath];
		return;
	}
		
	if (dirMap.find (row[dirColumns.realpath] + tmp + "/") == dirMap.end ())
	{
		newRow = *(dirStore->append(row.children()));
		newRow[dirColumns.name] = WUtil::ConvertToUTF8(tmp);
		newRow[dirColumns.realpath] = row[dirColumns.realpath] + 
			WUtil::ConvertToUTF8(tmp) + "/";
		dirMap[newRow[dirColumns.realpath]] = newRow;
		addDir (getRemainingDir (path), newRow, current);
	}
	else
		addDir (getRemainingDir (path), dirMap[row[dirColumns.realpath] + tmp + "/"], current);
}
void DownloadQueue::addFile (QueueItemInfo *i, ustring path)
{
	queueSize+=i->getSize();
	queueItems++;
	dirFileMap[path].push_back (i);
}

void DownloadQueue::buttonPressedDir (GdkEventButton *event)
{
	dirPrevious = event->type;
}
void DownloadQueue::buttonReleasedDir (GdkEventButton *event)
{
	//single click
	if (dirPrevious == GDK_BUTTON_PRESS) {
		//left button
		if (event->button == 1)
		{
			update();
		}

		//right button
		if (event->button == 3)
		{
			dirMenu.popup(event->button, event->time);
			dirMenu.show_all();
		}
	}
}

void DownloadQueue::buttonPressedFile (GdkEventButton *event)
{
	filePrevious = event->type;
}
void DownloadQueue::buttonReleasedFile (GdkEventButton *event)
{
	//single click
	if (filePrevious == GDK_BUTTON_PRESS)
	{

		//right button
		if (event->button == 3)
		{
			rebuildMenu ();
			fileMenu.popup(event->button, event->time);
			fileMenu.show_all();
		}
	}
}
string DownloadQueue::getTrailingSubDir (string path)
{
	if (path == "")
		return "";
	string tmp = path.substr (0, path.find_last_of ('/', path.size ()-2));
	return tmp + "/";
}
void DownloadQueue::removeFiles (ustring path)
{
	vector<QueueItemInfo*>::iterator it;
	for (it = dirFileMap[path].begin(); it != dirFileMap[path].end(); it++)
	{
		QueueManager::getInstance ()->remove ((*it)->getTarget());
	}
}
void DownloadQueue::getChildren (ustring path, vector<TreeModel::iterator> *iter)
{
	if (dirMap.find (path) == dirMap.end ())
		return;
	for (TreeModel::iterator it=dirMap[path].children ().begin (); it != dirMap[path].children ().end (); it++)
	{
		iter->push_back (it);
		getChildren ((*it)[dirColumns.realpath], iter);
	}
}
void DownloadQueue::removeDirClicked ()
{
	TreeModel::Row r = *(dirView.get_selection()->get_selected());

	if (!r)
		return;


	vector<TreeModel::iterator> iters;
	getChildren (r[dirColumns.realpath], &iters);
	for (int i=iters.size ()-1; i>=0; i--)
		removeFiles ((*iters[i])[dirColumns.realpath]);
		
	if (r)
		removeFiles (r[dirColumns.realpath]);

	update ();
}
void DownloadQueue::removeFileClicked ()
{
	TreeModel::Row r = *(fileView.get_selection()->get_selected());

	if (!r)
		return;

		
	QueueManager::getInstance ()->remove (((QueueItemInfo*)r[fileColumns.item])->getTarget ());
}
int DownloadQueue::countFiles (Glib::ustring path)
{
	if (!dirFileMap[path].empty ())
		return 1;
		
	vector<TreeModel::iterator> iter;
	getChildren (path, &iter);
	for (int i=0; i<iter.size ();i++)
		if (!dirFileMap[(*iter[i])[dirColumns.realpath]].empty ())
			return 1;

	return 0;
}
void DownloadQueue::removeDir (ustring path)
{
	if (path == "" || path == "/")
		return;

	if (countFiles (path) == 0)
	{
		if (dirFileMap.find (path) != dirFileMap.end ())
		{
			if (showingDir == path)
				fileStore->clear ();

			dirFileMap.erase (dirFileMap.find (path));
		}
		if (dirMap.find (path) != dirMap.end ())
		{
			dirStore->erase (*dirMap[path]);
			dirMap.erase (dirMap.find (path));
		}
		removeDir (getTrailingSubDir (path));
	}
	else
		return;
}
void DownloadQueue::removeFile (Glib::ustring target)
{
	ustring path = Util::getFilePath(target.raw());
	if (dirFileMap.find (path) == dirFileMap.end ())
		return;
	for (vector<QueueItemInfo*>::iterator it=dirFileMap[path].begin (); it != dirFileMap[path].end (); it++)
		if ((*it)->getTarget() == target)
		{
			queueSize-=(*it)->getSize();
			queueItems--;
			if (fileMap.find (target) != fileMap.end ())
				fileMap.erase (fileMap.find (target));
			dirFileMap[path].erase (it);
			break;
		}
	if (dirFileMap[path].empty ())
		removeDir (path);
	update ();
}
void DownloadQueue::on(QueueManagerListener::Added, QueueItem* aQI) throw()
{
	TreeModel::Row row;
	ustring subdir = getNextSubDir (Util::getFilePath(aQI->getTarget()));
	QueueItemInfo *i;
	if (dirMap.find ("/" +subdir  + "/") == dirMap.end ())
	{
		row = *(dirStore->append());
		row[dirColumns.name] = WUtil::ConvertToUTF8(subdir);
		row[dirColumns.realpath] = "/" + WUtil::ConvertToUTF8(subdir) + "/";
		dirMap[row[dirColumns.realpath]] = row;
		ustring tmp;
		addDir (getRemainingDir (Util::getFilePath(aQI->getTarget())), row, tmp);
		fileMap[aQI->getTarget ()] = aQI;
		i = new QueueItemInfo (aQI);
		addFile (i, tmp);
	}
	else
	{
		ustring tmp;
		addDir (getRemainingDir (Util::getFilePath(aQI->getTarget())),
						dirMap["/" + subdir + "/"], tmp);
		fileMap[aQI->getTarget ()] = aQI;
		i = new QueueItemInfo (aQI);
		addFile (i, tmp);
	}

	TreeModel::Row r = *(dirView.get_selection()->get_selected());

	if (!r)
		return;
	
	if (showingDir == r[dirColumns.realpath])
		i->update (this, true);
}
void DownloadQueue::on(QueueManagerListener::Finished, QueueItem* aQI) throw()
{

}
void DownloadQueue::on(QueueManagerListener::Moved, QueueItem* aQI) throw()
{
}
void DownloadQueue::on(QueueManagerListener::Removed, QueueItem* aQI) throw()
{
	Lock l(cs);
	removeFile (aQI->getTarget());
	update ();
}

void DownloadQueue::updateFiles (QueueItem *aQI)
{
	QueueItemInfo* ii = NULL;
	Lock l(cs);
	ustring path = Util::getFilePath (aQI->getTarget ());
	if (dirFileMap.find (path) == dirFileMap.end ())
		return;
	for (vector<QueueItemInfo*>::iterator it=dirFileMap[path].begin (); it != dirFileMap[path].end (); it++)
		if ((*it)->getTarget () == aQI->getTarget ())
		{
			ii = *it;
			break;
		}

	if (!ii)
		return;

	ii->setPriority(aQI->getPriority());
	ii->setStatus(aQI->getStatus());
	ii->setDownloadedBytes(aQI->getDownloadedBytes());
	ii->setTTH(aQI->getTTH());

	for(QueueItemInfo::SourceIter i = ii->getSources().begin(); i != ii->getSources().end(); )
	{
		if(!aQI->isSource(i->getUser()))
			i = ii->getSources().erase(i);
		else
			++i;
	}
	for(QueueItem::Source::Iter j = aQI->getSources().begin(); j != aQI->getSources().end(); ++j)
		if(!ii->isSource((*j)->getUser()))
			ii->getSources().push_back(QueueItemInfo::SourceInfo(*(*j)));
	for(QueueItemInfo::SourceIter i = ii->getBadSources().begin(); i != ii->getBadSources().end(); )
	{
		if(!aQI->isBadSource(i->getUser()))
			i = ii->getBadSources().erase(i);
		else
			++i;
	}
	for(QueueItem::Source::Iter j = aQI->getBadSources().begin(); j != aQI->getBadSources().end(); ++j)
		if(!ii->isBadSource((*j)->getUser()))
			ii->getBadSources().push_back(QueueItemInfo::SourceInfo(*(*j)));

	ii->update (this, false);
	updateStatus ();
}

void DownloadQueue::setFilePriority (Glib::ustring target, QueueItem::Priority p)
{
	QueueManager::getInstance ()->setPriority(WUtil::ConvertFromUTF8(target), p);
}
void DownloadQueue::setDirPriority (Glib::ustring path, QueueItem::Priority p)
{
	vector<TreeModel::iterator> iter;
	getChildren (path, &iter);
	for (int i=0;i<iter.size ();i++)
		for (vector<QueueItemInfo*>::iterator it=dirFileMap[(*iter[i])[dirColumns.realpath]].begin (); it != dirFileMap[(*iter[i])[dirColumns.realpath]].end (); it++)
			setFilePriority ((*it)->getTarget (), p);
}
