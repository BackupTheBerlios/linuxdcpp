#include "stdinc.h"
#include "DCPlusPlus.h"
#include "ResourceManager.h"
string ResourceManager::strings[] = {
"Active", 
"Enabled / Search String", 
"&Add", 
"Added", 
"Add finished files to share instantly (if shared)", 
"Add To Favorites", 
"Address already in use", 
"Address not available", 
"Discard", 
"Automatic Directory Listing Search", 
"All download slots taken", 
"All %d users offline", 
"All 3 users offline", 
"All 4 users offline", 
"Any", 
"At least", 
"At most", 
"Audio", 
"Auto connect / Name", 
"Auto grant slot / Nick", 
"Average/s: ", 
"AWAY", 
"Away mode off", 
"Away mode on: ", 
"B", 
"Both users offline", 
"Browse...", 
"&Browse...", 
"Choose folder", 
"Close connection", 
"Closing connection...", 
"Close", 
"Compressed", 
"Error during compression", 
"&Connect", 
"Connected", 
"Connecting...", 
"Connecting (forced)...", 
"Connecting to ", 
"Connection", 
"Connection closed", 
"Connection refused by target machine", 
"Connection reset by server", 
"Connection timeout", 
"Copy address to clipboard", 
"Copy magnet link to clipboard", 
"Copy nick to clipboard", 
"Could not open target file: ", 
"Count", 
"CRC Checked", 
"Error during decompression", 
"Description", 
"Destination", 
"Directory", 
"Directory already shared", 
"Disconnected", 
"Disconnected user leaving the hub: ", 
"Disk full(?)", 
"Document", 
"Done", 
"The temporary download directory cannot be shared", 
"Download", 
"Download failed: ", 
"Download finished, idle...", 
"Download starting...", 
"Download to...", 
"Download Queue", 
"Download whole directory", 
"Download whole directory to...", 
"Downloaded", 
"Downloaded %s (%.01f%%) in %s", 
" downloaded from ", 
"Downloading...", 
"Downloading public hub list...", 
"Downloads", 
"Duplicate file will not be shared: ", 
"Dupe matched against: ", 
"Duplicate source", 
"Edit", 
"E-Mail", 
"Please enter a nickname in the settings dialog!", 
"Please enter a password", 
"Please enter a reason", 
"Enter search string", 
"Please enter a destination server", 
"Errors", 
"Exact size", 
"Executable", 
"Favorite Hubs", 
"Favorite hub added", 
"Favorite Users", 
"Favorite user added", 
"Join/part of favorite users showing off", 
"Join/part of favorite users showing on", 
"File", 
"Files", 
"File list refresh finished", 
"File list refresh initiated", 
"File not available", 
"File type", 
"A file with a different size already exists in the queue", 
"A file with diffent tth root already exists in the queue", 
"Filename", 
"F&ilter", 
"Find", 
"Finished Downloads", 
"Finished Uploads", 
"Directory with '$' cannot be downloaded and will not be shared: ", 
"File with '$' cannot be downloaded and will not be shared: ", 
"Force attempt", 
"GiB", 
"Get file list", 
"Go to directory", 
"Grant extra slot", 
"Hash database", 
"Hash database rebuilt", 
"Finished hashing ", 
"High", 
"Highest", 
"Hit Ratio: ", 
"Hits: ", 
"Host unreachable", 
"Hub", 
"Hubs", 
"Address", 
"Hub list downloaded...", 
"Name", 
"Hub password", 
"Users", 
"Ignored message: ", 
"Invalid number of slots", 
"Invalid target file (missing directory, check default download directory setting)", 
"Downloaded tree does not match TTH root", 
"IP: ", 
"IP", 
"Items", 
"Joins: ", 
"Join/part showing off", 
"Join/part showing on", 
"KiB", 
"KiB/s", 
"Kick user(s)", 
"A file of equal or larger size already exists at the target location", 
"Last change: ", 
"Hub (last seen on if offline)", 
"Time last seen", 
"Loading DC++, please wait...", 
"Lookup TTH at Bitzi.com", 
"Low", 
"Lowest", 
"Filename:", 
"File Hash:", 
"Do nothing", 
"Add this file to your download queue", 
"Do the same action next time without asking", 
"Start a search for this file", 
"A MAGNET link was given to DC++, but it didn't contain a valid file hash for use on the Direct Connect network.  No action will be taken.", 
"DC++ has detected a MAGNET link with a file hash that can be searched for on the Direct Connect network.  What would you like to do?", 
"MAGNET Link detected", 
"Download files from the Direct Connect network", 
"DC++", 
"URL:MAGNET URI", 
"Manual connect address", 
"Match queue", 
"Matched %d file(s)", 
"MiB", 
"MiB/s", 
"&File", 
"ADL Search", 
"&Download Queue\tCtrl+D", 
"&Exit", 
"&Favorite Hubs\tCtrl+F", 
"Favorite &Users\tCtrl+U", 
"Follow last redirec&t\tCtrl+T", 
"Network Statistics", 
"&Notepad\tCtrl+N", 
"Open file list...", 
"&Public Hubs\tCtrl+P", 
"&Quick Connect ...\tCtrl+Q", 
"&Reconnect\tCtrl+R", 
"Refresh file list\tCtrl+E", 
"Show", 
"&Search\tCtrl+S", 
"Search Spy", 
"Settings...", 
"&Help", 
"About DC++...", 
"Change Log", 
"DC++ discussion forum", 
"Donate ���/$$$ (paypal)", 
"Downloads", 
"Translations", 
"Frequently asked questions", 
"Help forum", 
"DC++ Homepage", 
"Open downloads directory", 
"Readme / Newbie help", 
"Request a feature", 
"Report a bug", 
"&View", 
"&Status bar\tCtrl+2", 
"&Toolbar\tCtrl+1", 
"T&ransfers\tCtrl+3", 
"&Window", 
"Arrange icons", 
"Cascade", 
"Horizontal Tile", 
"Vertical Tile", 
"Close disconnected", 
"Minimize &All", 
"Move/Rename", 
"Move &Down", 
"Move &Up", 
"Network Statistics", 
"Network unreachable (are you connected to the internet?)", 
"Next", 
"&New...", 
"Nick", 
"Your nick was already taken, please change to something else!", 
" (Nick unknown)", 
"Non-blocking operation still in progress", 
"Not connected", 
"Not a socket", 
"No", 
"No directory specified", 
"You're trying to download from yourself!", 
"No errors", 
"No matches", 
"No slots available", 
"No users", 
"No users to download from", 
"Normal", 
"Notepad", 
"Offline", 
"Online", 
"Only users with free slots", 
"Only where I'm op", 
"Open", 
"Open download page?", 
"Open folder", 
"Operation would block execution", 
"Out of buffer space", 
"Passive user", 
"Password", 
"Parts: ", 
"Path", 
"Paused", 
"Permission denied", 
"Picture", 
"Port: ", 
"Port %d is busy, please choose another one in the settings dialog, or disable any other application that might be using it and restart DC++", 
"Preparing file list...", 
"Press the follow redirect button to connect to ", 
"Priority", 
"Private message from ", 
"&Properties", 
"Public Hubs", 
"Quick Connect", 
"Ratio", 
"Re-add source", 
"Really exit?", 
"Redirect", 
"Redirect request received to a hub that's already connected", 
"Redirect user(s)", 
"&Refresh", 
"Refresh user list", 
"&Remove", 
"Remove all", 
"Remove all subdirectories before adding this one", 
"Remove user from queue", 
"Remove source", 
"Rollback inconsistency, existing file does not match the one being downloaded", 
"Running...", 
"Search", 
"Search by TTH", 
"Search for", 
"Search for alternates", 
"Search for file", 
"Search options", 
"Search spam detected from ", 
"Search Spy", 
"Search String", 
"Specifying the same search string for more than 5 files for a passive connection or 10 files for an active connection is inefficient. Would you like to continue with the operation?", 
"Searching for ", 
"Send private message", 
"Separator", 
"Server", 
"Set priority", 
"&Add folder", 
"Break on first ADLSearch match", 
"Advanced", 
"Advanced settings", 
"Use antifragmentation method for downloads", 
"Appearance", 
"Auto-away on minimize (and back on restore)", 
"Automatically follow redirects", 
"Automatically disconnect users who leave the hub (does not disconnect when hub goes down / you leave it)", 
"Automatically search for alternative download locations", 
"Automatically match queue for auto search hits", 
"Use default search strings in auto search when no string is specified", 
"Automatically refresh share list every hour", 
"&Change", 
"Clear search box after each search", 
"Colors", 
"Enable safe and compressed transfers", 
"Command", 
"Confirm application exit", 
"Connection Settings (see the readme / newbie help if unsure)", 
"Connection Type", 
"Default away message", 
"Directories", 
"Default download directory", 
"Limits", 
"Downloads", 
"Maximum simultaneous downloads (0 = infinite)", 
"No new downloads if speed exceeds (KiB/s, 0 = disable)", 
"Donate ���:s! (ok, dirty dollars are fine as well =) (see help menu)", 
"Only show joins / parts for favorite users", 
"Filter kick and NMDC debug messages", 
"Set Finished Manager(s) tab bold when an entry is added", 
"Format", 
"General", 
"Accept custom user commands from hub", 
"Ignore private messages from offline users", 
"IP", 
"Don't delete file lists when exiting", 
"Language file", 
"Keep duplicate files in your file list (duplicates never count towards your share size)", 
"Log downloads", 
"Log filelist transfers", 
"Log main chat", 
"Log private chat", 
"Log uploads", 
"Log system messages", 
"Logging", 
"Logs and Sound", 
"Ask what to do when a magnet link is detected.", 
"Max hash speed", 
"Max tab rows", 
"Minimize to tray", 
"Name", 
"Don't send the away message to bots", 
"Note; New files are added to the share only once they've been hashed!", 
"Open the favorite hubs window at startup", 
"Open the finished downloads window at startup", 
"Open the public hubs window at startup", 
"Open the download queue window at startup", 
"Options", 
"Passive", 
"Personal Information", 
"Make an annoying sound every time a private message is received", 
"Make an annoying sound when a private message window is opened", 
"Open new file list windows in the background", 
"Open new private message windows in the background", 
"Open private messages from offline users in their own window", 
"Open private messages in their own window", 
"Port (empty=random)", 
"Public Hubs list", 
"Public Hubs list URL", 
"HTTP Proxy (for hublist only)", 
"Set Download Queue tab bold when contents change", 
"Note; most of these options require that you restart DC++", 
"Rollback", 
"Select &text style", 
"Select &window color", 
"Send unknown /commands to the hub", 
"Enable automatic SFV checking", 
"Share hidden files", 
"Total size:", 
"Shared directories", 
"Show joins / parts in chat by default", 
"Show progress bars for transfers (uses some CPU)", 
"Skip zero-byte files", 
"Use small send buffer (enable if uploads slow downloads a lot)", 
"SOCKS5", 
"Socks IP", 
"Port", 
"Use SOCKS5 server to resolve hostnames", 
"Username", 
"Sounds", 
"Note; because of changing download speeds, this is not 100% accurate...", 
"View status messages in main chat", 
"Tab completion of nicks in chat", 
"Set hub/PM tab bold when contents change", 
"Show timestamps in chat by default", 
"Set timestamps", 
"Unfinished downloads directory", 
"Sharing", 
"Automatically open an extra slot if speed is below (0 = disable)", 
"Upload slots", 
"Register with Windows to handle dchub:// URL links", 
"Register with Windows to handle magnet: URI links", 
"Use OEM monospaced font for viewing text files", 
"Use system icons when browsing files (slows browsing down a bit)", 
"Advanced\\User Commands", 
"Write buffer size", 
"Get User Country", 
"Log status messages", 
"Settings", 
"CRC32 inconsistency (SFV-Check)", 
"Shared", 
"Shared Files", 
"Size", 
"Max Size", 
"Min Size", 
"Slot granted", 
"Slots", 
"Slots set", 
"Socket has been shut down", 
"Socks server authentication failed (bad username / password?)", 
"The socks server doesn't support user / password authentication", 
"The socks server failed establish a connection", 
"The socks server requires authentication", 
"Failed to set up the socks server for UDP relay (check socks address and port)", 
"Source Type", 
"Specify a server to connect to", 
"Specify a search string", 
"Speed", 
"Status", 
"Stored password sent...", 
"Tag", 
"Target filename too long", 
"TiB", 
"Time", 
"Time left", 
"Timestamps disabled", 
"Timestamps enabled", 
"Total: ", 
"TTH inconsistency", 
"TTH Root", 
"Type", 
"Unable to create thread", 
"Unknown", 
"Unknown address", 
"Unknown command: ", 
"Unknown error: 0x%x", 
"Unsupported filelist format", 
"Upload finished, idle...", 
"Upload starting...", 
"Uploaded %s (%.01f%%) in %s", 
" uploaded to ", 
"Uploads", 
"User", 
"User Description", 
"User offline", 
"User went offline", 
"Users", 
"Video", 
"View as text", 
"Waiting...", 
"Waiting (User online)", 
"Waiting (%d of %d users online)", 
"Waiting to retry...", 
"What's &this?", 
"Yes", 
"You are being redirected to ", 
};
string ResourceManager::names[] = {
"Active", 
"ActiveSearchString", 
"Add", 
"Added", 
"AddFinishedInstantly", 
"AddToFavorites", 
"AddressAlreadyInUse", 
"AddressNotAvailable", 
"AdlDiscard", 
"AdlSearch", 
"AllDownloadSlotsTaken", 
"AllUsersOffline", 
"All3UsersOffline", 
"All4UsersOffline", 
"Any", 
"AtLeast", 
"AtMost", 
"Audio", 
"AutoConnect", 
"AutoGrant", 
"Average", 
"Away", 
"AwayModeOff", 
"AwayModeOn", 
"B", 
"BothUsersOffline", 
"Browse", 
"BrowseAccel", 
"ChooseFolder", 
"CloseConnection", 
"ClosingConnection", 
"Close", 
"Compressed", 
"CompressionError", 
"Connect", 
"Connected", 
"Connecting", 
"ConnectingForced", 
"ConnectingTo", 
"Connection", 
"ConnectionClosed", 
"ConnectionRefused", 
"ConnectionReset", 
"ConnectionTimeout", 
"CopyHub", 
"CopyMagnet", 
"CopyNick", 
"CouldNotOpenTargetFile", 
"Count", 
"CrcChecked", 
"DecompressionError", 
"Description", 
"Destination", 
"Directory", 
"DirectoryAlreadyShared", 
"Disconnected", 
"DisconnectedUser", 
"DiscFull", 
"Document", 
"Done", 
"DontShareTempDirectory", 
"Download", 
"DownloadFailed", 
"DownloadFinishedIdle", 
"DownloadStarting", 
"DownloadTo", 
"DownloadQueue", 
"DownloadWholeDir", 
"DownloadWholeDirTo", 
"Downloaded", 
"DownloadedBytes", 
"DownloadedFrom", 
"Downloading", 
"DownloadingHubList", 
"Downloads", 
"DuplicateFileNotShared", 
"DuplicateMatch", 
"DuplicateSource", 
"Edit", 
"Email", 
"EnterNick", 
"EnterPassword", 
"EnterReason", 
"EnterSearchString", 
"EnterServer", 
"Errors", 
"ExactSize", 
"Executable", 
"FavoriteHubs", 
"FavoriteHubAdded", 
"FavoriteUsers", 
"FavoriteUserAdded", 
"FavJoinShowingOff", 
"FavJoinShowingOn", 
"File", 
"Files", 
"FileListRefreshFinished", 
"FileListRefreshInitiated", 
"FileNotAvailable", 
"FileType", 
"FileWithDifferentSize", 
"FileWithDifferentTth", 
"Filename", 
"Filter", 
"Find", 
"FinishedDownloads", 
"FinishedUploads", 
"ForbiddenDollarDirectory", 
"ForbiddenDollarFile", 
"ForceAttempt", 
"Gb", 
"GetFileList", 
"GoToDirectory", 
"GrantExtraSlot", 
"HashDatabase", 
"HashRebuilt", 
"HashingFinished", 
"High", 
"Highest", 
"HitRatio", 
"Hits", 
"HostUnreachable", 
"Hub", 
"Hubs", 
"HubAddress", 
"HubListDownloaded", 
"HubName", 
"HubPassword", 
"HubUsers", 
"IgnoredMessage", 
"InvalidNumberOfSlots", 
"InvalidTargetFile", 
"InvalidTree", 
"Ip", 
"IpBare", 
"Items", 
"Joins", 
"JoinShowingOff", 
"JoinShowingOn", 
"Kb", 
"Kbps", 
"KickUser", 
"LargerTargetFileExists", 
"LastChange", 
"LastHub", 
"LastSeen", 
"Loading", 
"LookupAtBitzi", 
"Low", 
"Lowest", 
"MagnetDlgFile", 
"MagnetDlgHash", 
"MagnetDlgNothing", 
"MagnetDlgQueue", 
"MagnetDlgRemember", 
"MagnetDlgSearch", 
"MagnetDlgTextBad", 
"MagnetDlgTextGood", 
"MagnetDlgTitle", 
"MagnetHandlerDesc", 
"MagnetHandlerRoot", 
"MagnetShellDesc", 
"ManualAddress", 
"MatchQueue", 
"MatchedFiles", 
"Mb", 
"Mbps", 
"MenuFile", 
"MenuAdlSearch", 
"MenuDownloadQueue", 
"MenuExit", 
"MenuFavoriteHubs", 
"MenuFavoriteUsers", 
"MenuFollowRedirect", 
"MenuNetworkStatistics", 
"MenuNotepad", 
"MenuOpenFileList", 
"MenuPublicHubs", 
"MenuQuickConnect", 
"MenuReconnect", 
"MenuRefreshFileList", 
"MenuShow", 
"MenuSearch", 
"MenuSearchSpy", 
"MenuSettings", 
"MenuHelp", 
"MenuAbout", 
"MenuChangelog", 
"MenuDiscuss", 
"MenuDonate", 
"MenuHelpDownloads", 
"MenuHelpTranslations", 
"MenuFaq", 
"MenuHelpForum", 
"MenuHomepage", 
"MenuOpenDownloadsDir", 
"MenuReadme", 
"MenuRequestFeature", 
"MenuReportBug", 
"MenuView", 
"MenuStatusBar", 
"MenuToolbar", 
"MenuTransferView", 
"MenuWindow", 
"MenuArrange", 
"MenuCascade", 
"MenuHorizontalTile", 
"MenuVerticalTile", 
"MenuCloseDisconnected", 
"MenuMinimizeAll", 
"Move", 
"MoveDown", 
"MoveUp", 
"NetworkStatistics", 
"NetworkUnreachable", 
"Next", 
"New", 
"Nick", 
"NickTaken", 
"NickUnknown", 
"NonBlockingOperation", 
"NotConnected", 
"NotSocket", 
"No", 
"NoDirectorySpecified", 
"NoDownloadsFromSelf", 
"NoErrors", 
"NoMatches", 
"NoSlotsAvailable", 
"NoUsers", 
"NoUsersToDownloadFrom", 
"Normal", 
"Notepad", 
"Offline", 
"Online", 
"OnlyFreeSlots", 
"OnlyWhereOp", 
"Open", 
"OpenDownloadPage", 
"OpenFolder", 
"OperationWouldBlockExecution", 
"OutOfBufferSpace", 
"PassiveUser", 
"Password", 
"Parts", 
"Path", 
"Paused", 
"PermissionDenied", 
"Picture", 
"Port", 
"PortIsBusy", 
"PreparingFileList", 
"PressFollow", 
"Priority", 
"PrivateMessageFrom", 
"Properties", 
"PublicHubs", 
"QuickConnect", 
"Ratio", 
"ReaddSource", 
"ReallyExit", 
"Redirect", 
"RedirectAlreadyConnected", 
"RedirectUser", 
"Refresh", 
"RefreshUserList", 
"Remove", 
"RemoveAll", 
"RemoveAllSubdirectories", 
"RemoveFromAll", 
"RemoveSource", 
"RollbackInconsistency", 
"Running", 
"Search", 
"SearchByTth", 
"SearchFor", 
"SearchForAlternates", 
"SearchForFile", 
"SearchOptions", 
"SearchSpamFrom", 
"SearchSpy", 
"SearchString", 
"SearchStringInefficient", 
"SearchingFor", 
"SendPrivateMessage", 
"Separator", 
"Server", 
"SetPriority", 
"SettingsAddFolder", 
"SettingsAdlsBreakOnFirst", 
"SettingsAdvanced", 
"SettingsAdvancedSettings", 
"SettingsAntiFrag", 
"SettingsAppearance", 
"SettingsAutoAway", 
"SettingsAutoFollow", 
"SettingsAutoKick", 
"SettingsAutoSearch", 
"SettingsAutoSearchAutoMatch", 
"SettingsAutoSearchAutoString", 
"SettingsAutoUpdateList", 
"SettingsChange", 
"SettingsClearSearch", 
"SettingsColors", 
"SettingsCompressTransfers", 
"SettingsCommand", 
"SettingsConfirmExit", 
"SettingsConnectionSettings", 
"SettingsConnectionType", 
"SettingsDefaultAwayMsg", 
"SettingsDirectories", 
"SettingsDownloadDirectory", 
"SettingsDownloadLimits", 
"SettingsDownloads", 
"SettingsDownloadsMax", 
"SettingsDownloadsSpeedPause", 
"SettingsExampleText", 
"SettingsFavShowJoins", 
"SettingsFilterMessages", 
"SettingsFinishedDirty", 
"SettingsFormat", 
"SettingsGeneral", 
"SettingsHubUserCommands", 
"SettingsIgnoreOffline", 
"SettingsIp", 
"SettingsKeepLists", 
"SettingsLanguageFile", 
"SettingsListDupes", 
"SettingsLogDownloads", 
"SettingsLogFilelistTransfers", 
"SettingsLogMainChat", 
"SettingsLogPrivateChat", 
"SettingsLogUploads", 
"SettingsLogSystemMessages", 
"SettingsLogging", 
"SettingsLogs", 
"SettingsMagnetAsk", 
"SettingsMaxHashSpeed", 
"SettingsMaxTabRows", 
"SettingsMinimizeTray", 
"SettingsName", 
"SettingsNoAwaymsgToBots", 
"SettingsOnlyHashed", 
"SettingsOpenFavoriteHubs", 
"SettingsOpenFinishedDownloads", 
"SettingsOpenPublic", 
"SettingsOpenQueue", 
"SettingsOptions", 
"SettingsPassive", 
"SettingsPersonalInformation", 
"SettingsPmBeep", 
"SettingsPmBeepOpen", 
"SettingsPopunderFilelist", 
"SettingsPopunderPm", 
"SettingsPopupOffline", 
"SettingsPopupPms", 
"SettingsPort", 
"SettingsPublicHubList", 
"SettingsPublicHubListUrl", 
"SettingsPublicHubListHttpProxy", 
"SettingsQueueDirty", 
"SettingsRequiresRestart", 
"SettingsRollback", 
"SettingsSelectTextFace", 
"SettingsSelectWindowColor", 
"SettingsSendUnknownCommands", 
"SettingsSfvCheck", 
"SettingsShareHidden", 
"SettingsShareSize", 
"SettingsSharedDirectories", 
"SettingsShowJoins", 
"SettingsShowProgressBars", 
"SettingsSkipZeroByte", 
"SettingsSmallSendBuffer", 
"SettingsSocks5", 
"SettingsSocks5Ip", 
"SettingsSocks5Port", 
"SettingsSocks5Resolve", 
"SettingsSocks5Username", 
"SettingsSounds", 
"SettingsSpeedsNotAccurate", 
"SettingsStatusInChat", 
"SettingsTabCompletion", 
"SettingsTabDirty", 
"SettingsTimeStamps", 
"SettingsTimeStampsFormat", 
"SettingsUnfinishedDownloadDirectory", 
"SettingsUploads", 
"SettingsUploadsMinSpeed", 
"SettingsUploadsSlots", 
"SettingsUrlHandler", 
"SettingsUrlMagnet", 
"SettingsUseOemMonofont", 
"SettingsUseSystemIcons", 
"SettingsUserCommands", 
"SettingsWriteBuffer", 
"SettingsGetUserCountry", 
"SettingsLogStatusMessages", 
"Settings", 
"SfvInconsistency", 
"Shared", 
"SharedFiles", 
"Size", 
"SizeMax", 
"SizeMin", 
"SlotGranted", 
"Slots", 
"SlotsSet", 
"SocketShutDown", 
"SocksAuthFailed", 
"SocksAuthUnsupported", 
"SocksFailed", 
"SocksNeedsAuth", 
"SocksSetupError", 
"SourceType", 
"SpecifyServer", 
"SpecifySearchString", 
"Speed", 
"Status", 
"StoredPasswordSent", 
"Tag", 
"TargetFilenameTooLong", 
"Tb", 
"Time", 
"TimeLeft", 
"TimestampsDisabled", 
"TimestampsEnabled", 
"Total", 
"TthInconsistency", 
"TthRoot", 
"Type", 
"UnableToCreateThread", 
"Unknown", 
"UnknownAddress", 
"UnknownCommand", 
"UnknownError", 
"UnsupportedFilelistFormat", 
"UploadFinishedIdle", 
"UploadStarting", 
"UploadedBytes", 
"UploadedTo", 
"Uploads", 
"User", 
"UserDescription", 
"UserOffline", 
"UserWentOffline", 
"Users", 
"Video", 
"ViewAsText", 
"Waiting", 
"WaitingUserOnline", 
"WaitingUsersOnline", 
"WaitingToRetry", 
"WhatsThis", 
"Yes", 
"YouAreBeingRedirected", 
};
