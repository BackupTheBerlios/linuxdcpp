#ifndef WULFOR_FINISHED_TRANSFERS
#define WULFOR_FINISHED_TRANSFERS

#include "bookentry.hh"
#include "wulformanager.hh"
#include "callback.hh"
#include <client/FinishedManager.h>

class FinishedTransfers:
	public BookEntry,
	public FinishedManagerListener
{
	public:
	
	FinishedTransfers(int type, std::string title, GCallback closeCallback);
	~FinishedTransfers();
	
	GtkWidget *getWidget();
	
	void popupMenu_gui(GtkWidget *, GdkEventButton *, gpointer);
	void removeItems_gui(GtkMenuItem *, gpointer);
	void removeAll_gui(GtkMenuItem *, gpointer);
	void updateList(FinishedItem::List& list);
	void addEntry(FinishedItem *entry);
	void updateStatus();
	void openWith_gui(GtkMenuItem *, gpointer);
	
	//from FinishedManagerListener
	void on(AddedDl, FinishedItem* entry) throw();
	void on(AddedUl, FinishedItem* entry) throw();
	
	private:
	
	GtkWidget *mainBox;
	GtkListStore *transferStore;
	GtkTreeView *transferView;
	GtkTreeIter treeIter;
	GtkStatusbar *totalItems, *totalSize, *averageSpeed;
	GtkTreeSelection *transferSelection;
	
	int items;
	bool getType;
	int64_t totalBytes, totalTime;
	std::map<string, FinishedItem*> finishedList;
	
	GtkMenu *finishedTransfersMenu;
	GtkMenuItem *openWith, *remove, *removeAll;
	
	Callback3<FinishedTransfers, void, GtkWidget *, GdkEventButton *> menuCallback;
	Callback2<FinishedTransfers, void, GtkMenuItem *> removeCallback, removeAllCallback, openWithCallback;
	
	enum
	{
		COLUMN_TIME,
		COLUMN_FILENAME,
		COLUMN_PATH,
		COLUMN_NICK,
		COLUMN_HUB,
		COLUMN_SIZE,
		COLUMN_SPEED,
		COLUMN_CRC,
		COLUMN_TARGET
	};
	
};
#else
class FinishedTransfers;
#endif
