/**************************************************************************
 *  Far plugin helper (for FAR 2.0) (http://code.google.com/p/farplugs)   *
 *  Copyright (C) 2000-2011 by Artem Senichev <artemsen@gmail.com>        *
 *                                                                        *
 *  This program is free software: you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation, either version 3 of the License, or     *
 *  (at your option) any later version.                                   *
 *                                                                        *
 *  This program is distributed in the hope that it will be useful,       *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *  GNU General Public License for more details.                          *
 *                                                                        *
 *  You should have received a copy of the GNU General Public License     *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 **************************************************************************/

#pragma once

#include "FarPlugin.h"
#include "stdafx.h"

#define MAX_SIZE -1


/**
 * Far dialog wrapper
 */
class CFarDialog
{
public:
    /**
     * Constructor
     * \param width dialog width
     * \param height dialog height
     */
    explicit CFarDialog(const int width, const int height) :
        m_Dlg(INVALID_HANDLE_VALUE), _Width(width), _Height(height), _UseFrame(false)
    {
    }

    /**
     * Constructor
     * \param width dialog width
     * \param height dialog height
     * \param title dialog title
     */
    explicit CFarDialog(const int width, const int height, const wchar_t *title) :
        m_Dlg(INVALID_HANDLE_VALUE), _Width(width), _Height(height), _UseFrame(true)
    {
        SetTitle(title);
    }

    /**
     * Destructor
     */
    virtual ~CFarDialog()
    {
        if (m_Dlg != INVALID_HANDLE_VALUE)
        {
            CFarPlugin::GetPSI()->DialogFree(m_Dlg);
        }
    }

    /**
     * Get maximum dialogs width (FAR main window width)
     * \return maximum width
     */
    static int GetMaxWidth()
    {
        int wndWidth;

        SMALL_RECT rcFarWnd;
        if (CFarPlugin::AdvControl(ACTL_GETFARRECT, &rcFarWnd))
        {
            wndWidth = rcFarWnd.Right;
        }
        else
        {
            HANDLE stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
            CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
            wndWidth = (GetConsoleScreenBufferInfo(stdOut, &consoleInfo) ? consoleInfo.srWindow.Right : 79);
        }
        return wndWidth + 1;
    }

    /**
     * Get maximum dialogs height (FAR main window height)
     * \return maximum height
     */
    static int GetMaxHeight()
    {
        int wndHeight;

        SMALL_RECT rcFarWnd;
        if (CFarPlugin::AdvControl(ACTL_GETFARRECT, &rcFarWnd))
        {
            wndHeight = rcFarWnd.Bottom - rcFarWnd.Top;
        }
        else
        {
            HANDLE stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
            CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
            wndHeight = (GetConsoleScreenBufferInfo(stdOut, &consoleInfo) ? consoleInfo.srWindow.Bottom : 24);
        }
        return wndHeight + 1;
    }

    /**
     * Set dialog title and change window model
     * \param title dialog's title
     */
    inline void SetTitle(const wchar_t *title)
    {
        assert(title);
        assert(_DlgItems.empty());  //Must be first call!!!

        _UseFrame = true;
        CreateDlgItem(DI_DOUBLEBOX, 3, _Width - 4, 1, _Height - 2, title);
    }

    /**
     * Get top dialog coordinate
     * \return top dialog coordinate
     */
    inline int GetTop() const
    {
        return _UseFrame ? 2 : 0;
    }

    /**
     * Get left dialog coordinate
     * \return left dialog coordinate
     */
    inline int GetLeft() const
    {
        return _UseFrame ? 5 : 0;
    }

    /**
     * Get dialogs width
     * \return width
     */
    inline int GetWidth() const
    {
        return _Width - (_UseFrame ? 6 : 0);
    }

    /**
     * Get dialogs height
     * \return height
     */
    inline int GetHeight() const
    {
        return _Height - (_UseFrame ? 2 : 0);
    }

    /**
     * Resize dialog
     * \param width dialog width
     * \param height dialog height
     */
    inline void ResizeDialog(const int width, const int height)
    {
        assert(m_Dlg != INVALID_HANDLE_VALUE);
        _Width = width;
        _Height = height;
        COORD newSize;
        newSize.X = static_cast<SHORT>(width);
        newSize.Y = static_cast<SHORT>(height);
        CFarPlugin::GetPSI()->SendDlgMessage(m_Dlg, DM_RESIZEDIALOG, 0, reinterpret_cast<LONG_PTR>(&newSize));
    }

    /**
     * Show dialogs
     * \return -2 if error else dialog return code
     */
    inline int DoModal()
    {
        if (m_Dlg == INVALID_HANDLE_VALUE)
        {
            assert(!_DlgItems.empty());
            m_Dlg = CFarPlugin::GetPSI()->DialogInit(CFarPlugin::GetPSI()->ModuleNumber, -1, -1, _Width, _Height, NULL, &_DlgItems.front(), static_cast<unsigned int>(_DlgItems.size()), 0, 0, &CFarDialog::InternalDialogMessageProc, 0);
            _DlgItems.clear();  //Non actual for now
            if (m_Dlg == INVALID_HANDLE_VALUE)
            {
                return -2;
            }
        }

        AccessDlgInstances(m_Dlg, this);
        const int retCode = CFarPlugin::GetPSI()->DialogRun(m_Dlg);
        AccessDlgInstances(m_Dlg, this, true);

        return retCode;
    }

    /**
     * Redraw dialog
     */
    inline void Redraw()
    {
        assert(m_Dlg != INVALID_HANDLE_VALUE);
        CFarPlugin::GetPSI()->SendDlgMessage(m_Dlg, DM_REDRAW, 0, 0L);
    }

    /**
     * Move cursor position
     * \param dlgItemId item id
     * \param col horizontal coordinates (column)
     * \param row vertical coordinates (row)
     */
    inline void MoveCursor(const int dlgItemId, const int col, const int row)
    {
        assert(m_Dlg != INVALID_HANDLE_VALUE);
        COORD coord;
        coord.Y = static_cast<SHORT>(row);
        coord.X = static_cast<SHORT>(col);
        CFarPlugin::GetPSI()->SendDlgMessage(m_Dlg, DM_SETCURSORPOS, dlgItemId, reinterpret_cast<LONG_PTR>(&coord));
    }

    /**
     * Create dialog item
     * \param type dialog item type
     * \param colBegin column begin coordinate
     * \param colEnd column end coordinate
     * \param rowBegin row begin coordinate
     * \param rowEnd row end coordinate
     * \param ptrData data pointer
     * \param flags dialog item flags
     * \param dlgItem registered dialog item
     * \return dialog created dialogs item id
     */
    inline int CreateDlgItem(const int type, const int colBegin, const int colEnd,
                             const int rowBegin, const int rowEnd, const wchar_t *ptrData = NULL,
                             const DWORD flags = 0, FarDialogItem **dlgItem = NULL)
    {
        FarDialogItem item;
        ZeroMemory(&item, sizeof(item));

        item.Type = type;
        item.Flags = flags;
        item.X1 = colBegin;
        item.X2 = colEnd;
        item.Y1 = rowBegin;
        item.Y2 = rowEnd;
        item.PtrData = ptrData;

        _DlgItems.push_back(item);
        if (dlgItem)
        {
            *dlgItem = &_DlgItems.back();
        }
        return static_cast<int>(_DlgItems.size()) - 1;
    }

    /**
     * Create separator dialog item
     * \param row vertical coordinates (row)
     * \param title separator title
     * \return dialog created dialogs item id
     */
    inline int CreateSeparator(const int row, const wchar_t *title = NULL, const DWORD flags = 0, FarDialogItem **dlgItem = NULL)
    {
        return CreateDlgItem(DI_TEXT, 0, GetWidth(), row, row, title, DIF_SEPARATOR | flags, dlgItem);
    }

    /**
     * Create text dialog item
     * \param col horizontal coordinates (column)
     * \param row vertical coordinates (row)
     * \param text dialog item text
     * \return dialog created dialogs item id
     */
    inline int CreateText(const int col, const int row, const wchar_t *text, const DWORD flags = 0, FarDialogItem **dlgItem = NULL)
    {
        return CreateDlgItem(DI_TEXT, col, col + lstrlen(text) - 1, row, row, text, flags, dlgItem);
    }

    /**
     * Create text dialog item (centered on dialog)
     * \param row vertical coordinates (row)
     * \param text dialog item text
     * \return dialog created dialogs item id
     */
    inline int CreateText(const int row, const wchar_t *text)
    {
        const int l = lstrlen(text);
        const int c = _Width / 2 - l / 2;
        return CreateDlgItem(DI_TEXT, c, c + l - 1, row, row, text);
    }

    /**
     * Create edit dialog item
     * \param col horizontal coordinates (column)
     * \param row vertical coordinates (row)
     * \param width item width (MAX_SIZE to full width)
     * \param text dialog item default text
     * \param history history path (NULL to ignore)
     * \param dlgItem registered dialog item
     * \return dialog created dialogs item id
     */
    inline int CreateEdit(const int col, const int row, const int width,
                          const wchar_t *text = NULL, const wchar_t *history = NULL,
                          const DWORD flags = 0, FarDialogItem **dlgItem = NULL)
    {
        FarDialogItem *dlgItemEdit;
        const int itemId = CreateDlgItem(DI_EDIT, col, width == MAX_SIZE ? GetWidth() : col + width,
                                         row, row, text, (history ? DIF_HISTORY : 0) | flags, &dlgItemEdit);
        if (history)
        {
            dlgItemEdit->History = history;
        }
        if (dlgItem)
        {
            *dlgItem = dlgItemEdit;
        }
        return itemId;
    }

    /**
     * Create button dialog item
     * \param col horizontal coordinates (column)
     * \param row vertical coordinates (row)
     * \param width button width
     * \param title button text
     * \param flags button flags
     * \param dlgItem registered dialog item
     * \return dialog created dialogs item id
     */
    inline int CreateButton(const int col, const int row, const wchar_t *title, const int flags = 0L, FarDialogItem **dlgItem = NULL)
    {
        return CreateDlgItem(DI_BUTTON, col, col, row, row, title, flags, dlgItem);
    }

    /**
     * Create radio button dialog item
     * \param col horizontal coordinates (column)
     * \param row vertical coordinates (row)
     * \param title button text
     * \param flags button flags
     * \param dlgItem registered dialog item
     * \return dialog created dialogs item id
     */
    inline int CreateRadioButton(const int col, const int row, const wchar_t *title = NULL, const int flags = 0L, FarDialogItem **dlgItem = NULL)
    {
        return CreateDlgItem(DI_RADIOBUTTON, col, 0, row, row, title, flags, dlgItem);
    }

    /**
     * Create checkbox dialog item
     * \param col horizontal coordinates (column)
     * \param row vertical coordinates (row)
     * \param title button text
     * \param initState initial state
     * \param flags button flags
     * \param dlgItem registered dialog item
     * \return dialog created dialogs item id
     */
    inline int CreateCheckBox(const int col, const int row, const wchar_t *title, const bool initState = false, const int flags = 0L, FarDialogItem **dlgItem = NULL)
    {
        FarDialogItem *dlgItemCB;
        const int itemId = CreateDlgItem(DI_CHECKBOX, col, 0, row, row, title, flags, &dlgItemCB);
        dlgItemCB->Selected = initState ? TRUE : FALSE;
        if (dlgItem)
        {
            *dlgItem = dlgItemCB;
        }
        return itemId;
    }

    /**
     * Get Far dialog item description
     * \param dlgItemId dialog item id
     * \return dialog item description. Must be deleted after using.
     */
    inline FarDialogItem *GetDlgItem(const int dlgItemId)
    {
        assert(m_Dlg != INVALID_HANDLE_VALUE);

        const size_t sz = CFarPlugin::GetPSI()->SendDlgMessage(m_Dlg, DM_GETDLGITEM, dlgItemId, 0L);
        assert(sz);
        FarDialogItem *item = reinterpret_cast<FarDialogItem *>(new unsigned char[sz]);
        CFarPlugin::GetPSI()->SendDlgMessage(m_Dlg, DM_GETDLGITEM, dlgItemId, reinterpret_cast<LONG_PTR>(item));
        return item;
    }

    /**
     * Set Far dialog item description
     * \param dlgItemId dialog item id
     * \param dlgItem dialog item description
     */
    inline void SetDlgItem(const int dlgItemId, const FarDialogItem &dlgItem)
    {
        assert(m_Dlg != INVALID_HANDLE_VALUE);
        CFarPlugin::GetPSI()->SendDlgMessage(m_Dlg, DM_SETDLGITEM, dlgItemId, reinterpret_cast<LONG_PTR>(&dlgItem));
    }

    /**
     * Show/hide dialog item
     * \param dlgItemId dialog item id
     * \param show true to show, false to hide
     */
    inline void ShowDlgItem(const int dlgItemId, const bool show)
    {
        assert(m_Dlg != INVALID_HANDLE_VALUE);
        CFarPlugin::GetPSI()->SendDlgMessage(m_Dlg, DM_SHOWITEM, dlgItemId, show ? 1 : 0);
    }

    /**
     * Get list/combobox selection index
     * \param dlgItemId item id
     * \return list/combobox selection index
     */
    inline int GetSelectonIndex(const int dlgItemId) const
    {
        assert(m_Dlg != INVALID_HANDLE_VALUE);
        return static_cast<int>(CFarPlugin::GetPSI()->SendDlgMessage(m_Dlg, DM_LISTGETCURPOS, dlgItemId, 0));
    }

    /**
     * Get checkbox/radiobutton state
     * \param dlgItemId item id
     * \return checkbox/radiobutton state
     */
    bool GetCheckState(const int dlgItemId) const
    {
        assert(m_Dlg != INVALID_HANDLE_VALUE);
        return CFarPlugin::GetPSI()->SendDlgMessage(m_Dlg, DM_GETCHECK, dlgItemId, 0) == BSTATE_CHECKED;
    }


    /**
     * Set checkbox/radiobutton state
     * \param dlgItemId item id
     * \param check checkbox/radiobutton state
     */
    void SetCheckState(const int dlgItemId, const bool check) const
    {
        assert(m_Dlg != INVALID_HANDLE_VALUE);
        CFarPlugin::GetPSI()->SendDlgMessage(m_Dlg, DM_SETCHECK, dlgItemId, check ? BSTATE_CHECKED : BSTATE_UNCHECKED);
    }

    /**
     * Get dialog item text
     * \param dlgItemId item id
     * \return item text
     */
    inline wstring GetText(const int dlgItemId) const
    {
        assert(m_Dlg != INVALID_HANDLE_VALUE);

        wstring itemText;

        const LONG_PTR itemTextLen = CFarPlugin::GetPSI()->SendDlgMessage(m_Dlg, DM_GETTEXTLENGTH, dlgItemId, 0);
        if (itemTextLen != 0)
        {
            itemText.resize(itemTextLen);
            FarDialogItemData item;
            item.PtrLength = itemTextLen;
            item.PtrData = &itemText[0];
            CFarPlugin::GetPSI()->SendDlgMessage(m_Dlg, DM_GETTEXT, dlgItemId, reinterpret_cast<LONG_PTR>(&item));
        }

        return itemText;
    }

    /**
     * Set dialog item text
     * \param dlgItemId item id
     * \param text item text
     */
    inline void SetText(const int dlgItemId, const wchar_t *text) const
    {
        assert(m_Dlg != INVALID_HANDLE_VALUE);

        FarDialogItemData item;
        item.PtrLength = wcslen(text);
        item.PtrData = const_cast<wchar_t *>(text);

        CFarPlugin::GetPSI()->SendDlgMessage(m_Dlg, DM_SETTEXT, dlgItemId, reinterpret_cast<LONG_PTR>(&item));
    }

protected:
    /**
     * Dialog message processing procedure
     * \param msg message id
     * \param param1 first parameter
     * \param param2 second parameter
     * \return code depending on message id
     */
    virtual LONG_PTR DialogMessageProc(int msg, int param1, LONG_PTR param2)
    {
        // DEBUG_PRINTF(L"NetBox: msg = %u, DN_BTNCLICK = %u, param1 = %u", msg, DN_BTNCLICK, param1);
        return CFarPlugin::GetPSI()->DefDlgProc(m_Dlg, msg, param1, param2);
    }

private:
    //! Dialog message handler procedure (see Far help)
    static LONG_PTR WINAPI InternalDialogMessageProc(HANDLE dlg, int msg, int param1, LONG_PTR param2)
    {
        CFarDialog *dlgInst = AccessDlgInstances(dlg, NULL);
        assert(dlgInst);
        return dlgInst->DialogMessageProc(msg, param1, param2);
    }

    /**
     * Access to dialog instances map
     * \param dlg Far dialog handle
     * \param inst dialog class instance
     * \param removeVal true to remove value
     * \return dialog class instance for handle
     */
    static CFarDialog *AccessDlgInstances(HANDLE dlg, CFarDialog *inst, const bool removeVal = false)
    {
        static map<HANDLE, CFarDialog *> dlgInstances;
        if (dlg && inst && !removeVal)
        {
            dlgInstances.insert(make_pair(dlg, inst));
            return NULL;
        }
        else if (removeVal)
        {
            dlgInstances.erase(dlg);
            return NULL;
        }

        map<HANDLE, CFarDialog *>::iterator it = dlgInstances.find(dlg);
        assert(it != dlgInstances.end());
        return it->second;
    }

protected:
    HANDLE                  m_Dlg;       ///< Dialog descriptor
    int                     _Width;     ///< Dialog width
    int                     _Height;    ///< Dialog height
    vector<FarDialogItem>   _DlgItems;  ///< Dialog items array
    bool                    _UseFrame;  ///< Dialog frame flag
};

//---------------------------------------------------------------------------
typedef void (*TThreadMethod)();
typedef void (*TNotifyEvent)(TObject *);
//---------------------------------------------------------------------------

class TFarDialogContainer;
class TFarDialogItem;
class TFarButton;
class TFarSeparator;
class TFarBox;
class TFarList;
struct FarDialogItem;
enum TItemPosition { ipNewLine, ipBelow, ipRight };
//---------------------------------------------------------------------------
typedef void (TObject::*TFarKeyEvent)
    (TFarDialog *Sender, TFarDialogItem *Item, long KeyCode, bool &Handled);
typedef void (TObject::*TFarMouseClickEvent) 
    (TFarDialogItem *Item, MOUSE_EVENT_RECORD *Event);
typedef void (TObject::*TFarProcessGroupEvent)(TFarDialogItem *Item, void *Arg);
//---------------------------------------------------------------------------
class TFarDialog : public TObject
{
    friend TFarDialogItem;
    friend TFarDialogContainer;
    friend TFarButton;
    friend TFarList;
    friend class TFarListBox;
    typedef TFarDialog self;
public:
    TFarDialog(TCustomFarPlugin *AFarPlugin);
    ~TFarDialog();

    int ShowModal();
    void ShowGroup(int Group, bool Show);
    void EnableGroup(int Group, bool Enable);

    TRect GetBounds() const { return FBounds; }
    void SetBounds(const TRect &value);
    TRect GetClientRect() const;
    wstring GetHelpTopic() const { return FHelpTopic; }
    void SetHelpTopic(const wstring &value);
    unsigned int GetFlags() const { return FFlags; }
    void SetFlags(const unsigned int &value);
    bool GetCentered() const;
    void SetCentered(const bool &value);
    TPoint GetSize() const;
    void SetSize(const TPoint &value);
    TPoint GetClientSize() const;
    int GetWidth() const;
    void SetWidth(const int &value);
    int GetHeight() const;
    void SetHeight(const int &value);
    wstring GetCaption() const;
    void SetCaption(const wstring &value);
    HANDLE GetHandle() const { return FHandle; }
    TFarButton *GetDefaultButton() const { return FDefaultButton; }
    TFarBox *GetBorderBox() const { return FBorderBox; }
    TFarDialogItem *GetItem(size_t Index) const;
    size_t GetItemCount() const;
    TItemPosition GetNextItemPosition() const { return FNextItemPosition; }
    void SetNextItemPosition(const TItemPosition &value) { FNextItemPosition = value; }
    void GetNextItemPosition(int &Left, int &Top);
    int GetDefaultGroup() const { return FDefaultGroup; }
    void SetDefaultGroup(const int &value) { FDefaultGroup = value; }
    int GetTag() const { return FTag; }
    void SetTag(const int &value) { FTag = value; }
    TFarDialogItem *GetItemFocused() const { return FItemFocused; }
    void SetItemFocused(TFarDialogItem * const &value);
    int GetResult() const { return FResult; }
    TPoint GetMaxSize() const;
    
    TFarKeyEvent GetOnKey() const { return FOnKey; }
    void SetOnKey(const TFarKeyEvent &value) { FOnKey = value; }

    void Redraw();
    void LockChanges();
    void UnlockChanges();
    char GetSystemColor(unsigned int Index);
    bool HotKey(unsigned long Key);

protected:
    // __property TCustomFarPlugin *FarPlugin = { read = FFarPlugin };
    property<self, TCustomFarPlugin *> FarPlugin;
    // __property TObjectList *Items = { read = FItems };
    property<self, TObjectList *> Items;

    void Add(TFarDialogItem *Item);
    void Add(TFarDialogContainer *Container);
    long SendMessage(int Msg, int Param1, int Param2);
    virtual long DialogProc(int Msg, int Param1, long Param2);
    virtual long FailDialogProc(int Msg, int Param1, long Param2);
    long DefaultDialogProc(int Msg, int Param1, long Param2);
    virtual bool MouseEvent(MOUSE_EVENT_RECORD *Event);
    virtual bool Key(TFarDialogItem *Item, long KeyCode);
    virtual void Change();
    virtual void Init();
    virtual bool CloseQuery();
    wstring GetMsg(int MsgId);
    void RefreshBounds();
    virtual void Idle();
    void BreakSynchronize();
    void Synchronize(TThreadMethod Method);
    void Close(TFarButton *Button);
    void ProcessGroup(int Group, TFarProcessGroupEvent Callback, void *Arg);
    void ShowItem(TFarDialogItem *Item, void *Arg);
    void EnableItem(TFarDialogItem *Item, void *Arg);
    bool ChangesLocked();
    TFarDialogItem *ItemAt(int X, int Y);

    static LONG_PTR WINAPI DialogProcGeneral(HANDLE Handle, int Msg, int Param1, LONG_PTR Param2);

private:
    TCustomFarPlugin *FFarPlugin;
    TRect FBounds;
    unsigned int FFlags;
    wstring FHelpTopic;
    bool FVisible;
    TObjectList *FItems;
    TObjectList *FContainers;
    HANDLE FHandle;
    TFarButton *FDefaultButton;
    TFarBox *FBorderBox;
    TItemPosition FNextItemPosition;
    int FDefaultGroup;
    int FTag;
    TFarDialogItem *FItemFocused;
    TFarKeyEvent FOnKey;
    FarDialogItem *FDialogItems;
    int FDialogItemsCapacity;
    int FChangesLocked;
    bool FChangesPending;
    int FResult;
    bool FNeedsSynchronize;
    HANDLE FSynchronizeObjects[2];
    TThreadMethod FSynchronizeMethod;
};
//---------------------------------------------------------------------------
class TFarDialogContainer : public TObject
{
    friend TFarDialog;
    friend TFarDialogItem;
    typedef TFarDialogContainer self;
public:
    // __property int Left = { read = FLeft, write = SetPosition, index = 0 };
    property<self, int> Left;
    // __property int Top = { read = FTop, write = SetPosition, index = 1 };
    property<self, int> Top;
    // __property bool Enabled = { read = FEnabled, write = SetEnabled };
    property<self, bool> Enabled;
    property_ro<self, size_t> ItemCount;

protected:
    TFarDialogContainer(TFarDialog *ADialog);
    ~TFarDialogContainer();

    // __property TFarDialog *Dialog = { read = FDialog };
    property<TFarDialogContainer, TFarDialog *> Dialog;

    void Add(TFarDialogItem *Item);
    void Remove(TFarDialogItem *Item);
    virtual void Change();
    wstring GetMsg(int MsgId);

private:
    int FLeft;
    int FTop;
    TObjectList *FItems;
    TFarDialog *FDialog;
    bool FEnabled;

    void SetPosition(int Index, int value);
    void SetEnabled(bool value);
    size_t GetItemCount() const;
};
//---------------------------------------------------------------------------
#define DIF_INVERSE 0x00000001UL
//---------------------------------------------------------------------------
class TFarDialogItem : public TObject
{
    friend TFarDialog;
    friend TFarDialogContainer;
    friend TFarList;
public:
    // __property TRect Bounds = { read = FBounds, write = SetBounds };
    property<TFarDialogItem, TRect> Bounds;
    // __property TRect ActualBounds = { read = GetActualBounds };
    property<TFarDialogItem, TRect> ActualBounds;
    // __property int Left = { read = GetCoordinate, write = SetCoordinate, index = 0 };
    property<TFarDialogItem, int> Left;
    // __property int Top = { read = GetCoordinate, write = SetCoordinate, index = 1 };
    property<TFarDialogItem, int> Top;
    // __property int Right = { read = GetCoordinate, write = SetCoordinate, index = 2 };
    property<TFarDialogItem, int> Right;
    // __property int Bottom = { read = GetCoordinate, write = SetCoordinate, index = 3 };
    property<TFarDialogItem, int> Bottom;
    // __property int Width = { read = GetWidth, write = SetWidth };
    property<TFarDialogItem, int> Width;
    // __property int Height = { read = GetHeight, write = SetHeight };
    property<TFarDialogItem, int> Height;
    // __property unsigned int Flags = { read = GetFlags, write = SetFlags };
    property<TFarDialogItem, unsigned int> Flags;
    // __property bool Enabled = { read = FEnabled, write = SetEnabled };
    property<TFarDialogItem, bool> Enabled;
    // __property bool IsEnabled = { read = FIsEnabled };
    property<TFarDialogItem, bool> IsEnabled;
    // __property TFarDialogItem *EnabledFollow = { read = FEnabledFollow, write = SetEnabledFollow };
    property<TFarDialogItem, TFarDialogItem *> EnabledFollow;
    // __property TFarDialogItem *EnabledDependency = { read = FEnabledDependency, write = SetEnabledDependency };
    property<TFarDialogItem, TFarDialogItem *> EnabledDependency;
    // __property TFarDialogItem *EnabledDependencyNegative = { read = FEnabledDependencyNegative, write = SetEnabledDependencyNegative };
    property<TFarDialogItem, TFarDialogItem *> EnabledDependencyNegative;
    // __property bool IsEmpty = { read = GetIsEmpty };
    property<TFarDialogItem, bool> IsEmpty;
    // __property int Group = { read = FGroup, write = FGroup };
    property<TFarDialogItem, int> Group;
    // __property bool Visible = { read = GetFlag, write = SetFlag, index = DIF_HIDDEN | DIF_INVERSE };
    property<TFarDialogItem, bool> Visible;
    // __property bool TabStop = { read = GetFlag, write = SetFlag, index = DIF_NOFOCUS | DIF_INVERSE };
    property<TFarDialogItem, bool> TabStop;
    // __property bool Oem = { read = FOem, write = FOem };
    property<TFarDialogItem, bool> Oem;
    // __property int Tag = { read = FTag, write = FTag };
    property<TFarDialogItem, int> Tag;
    // __property TFarDialog *Dialog = { read = FDialog };
    property<TFarDialogItem, TFarDialog *> Dialog;

    // __property TNotifyEvent OnExit = { read = FOnExit, write = FOnExit };
    property<TFarDialogItem, TNotifyEvent> OnExit;
    // __property TFarMouseClickEvent OnMouseClick = { read = FOnMouseClick, write = FOnMouseClick };
    property<TFarDialogItem, TFarMouseClickEvent> OnMouseClick;

    void Move(int DeltaX, int DeltaY);
    void MoveAt(int X, int Y);
    virtual bool CanFocus();
    bool Focused();
    void SetFocus();

protected:
    int FDefaultType;
    int FGroup;
    int FTag;
    TNotifyEvent FOnExit;
    TFarMouseClickEvent FOnMouseClick;

    TFarDialogItem(TFarDialog *ADialog, int AType);
    ~TFarDialogItem();

    // __property FarDialogItem *DialogItem = { read = GetDialogItem };
    property<TFarDialogItem, FarDialogItem *> DialogItem;
    // __property bool CenterGroup = { read = GetFlag, write = SetFlag, index = DIF_CENTERGROUP };
    property<TFarDialogItem, bool> CenterGroup;
    // __property wstring Data = { read = GetData, write = SetData };
    property<TFarDialogItem, wstring> Data;
    // __property int Type = { read = GetType, write = SetType };
    property<TFarDialogItem, int> Type;
    // __property int Item = { read = FItem };
    property<TFarDialogItem, int> Item;
    // __property int Selected = { read = GetSelected, write = SetSelected };
    property<TFarDialogItem, int> Selected;
    // __property TFarDialogContainer *Container = { read = FContainer, write = SetContainer };
    property<TFarDialogItem, TFarDialogContainer *> Container;
    // __property bool Checked = { read = GetChecked, write = SetChecked };
    property<TFarDialogItem, bool> Checked;

    virtual void Detach();
    void DialogResized();
    long SendMessage(int Msg, int Param);
    long SendDialogMessage(int Msg, int Param1, int Param2);
    virtual long ItemProc(int Msg, long Param);
    long DefaultItemProc(int Msg, int Param);
    long DefaultDialogProc(int Msg, int Param1, int Param2);
    virtual long FailItemProc(int Msg, long Param);
    virtual void Change();
    void DialogChange();
    void SetAlterType(int Index, bool value);
    bool GetAlterType(int Index);
    virtual void UpdateBounds();
    virtual void ResetBounds();
    virtual void Init();
    virtual bool CloseQuery();
    virtual bool MouseMove(int X, int Y, MOUSE_EVENT_RECORD *Event);
    virtual bool MouseClick(MOUSE_EVENT_RECORD *Event);
    TPoint MouseClientPosition(MOUSE_EVENT_RECORD *Event);
    void Text(int X, int Y, int Color, wstring Str, bool Oem = false);
    void Redraw();
    virtual bool HotKey(char HotKey);

    virtual bool GetIsEmpty();
    virtual void SetDataInternal(const wstring value);
    virtual void SetData(const wstring value);
    virtual wstring GetData();
    void UpdateData(const wstring value);
    void UpdateSelected(int value);

    bool GetFlag(int Index);
    void SetFlag(int Index, bool value);

    virtual void DoFocus();
    virtual void DoExit();

    char GetColor(int Index);
    void SetColor(int Index, char value);

private:
    TFarDialog *FDialog;
    TRect FBounds;
    TFarDialogItem *FEnabledFollow;
    TFarDialogItem *FEnabledDependency;
    TFarDialogItem *FEnabledDependencyNegative;
    TFarDialogContainer *FContainer;
    size_t FItem;
    bool FEnabled;
    bool FIsEnabled;
    unsigned long FColors;
    unsigned long FColorMask;
    bool FOem;

    void SetBounds(TRect value);
    void SetFlags(unsigned int value);
    void UpdateFlags(unsigned int value);
    TRect GetActualBounds();
    unsigned int GetFlags();
    void SetType(int value);
    int GetType();
    void SetEnabledFollow(TFarDialogItem *value);
    void SetEnabledDependency(TFarDialogItem *value);
    void SetEnabledDependencyNegative(TFarDialogItem *value);
    void SetSelected(int value);
    int GetSelected();
    void SetCoordinate(int Index, int value);
    int GetCoordinate(int Index);
    void SetWidth(int value);
    int GetWidth() const;
    void SetHeight(const int &value);
    int GetHeight() const;
    TFarDialogItem *GetPrevItem();
    void UpdateFocused(bool value);
    void SetContainer(const TFarDialogContainer *&value);
    void SetEnabled(const bool &value);
    void UpdateEnabled();
    void SetChecked(const bool &value);
    bool GetChecked() const;
    FarDialogItem *GetDialogItem() const;
};
//---------------------------------------------------------------------------
class TFarBox : public TFarDialogItem
{
public:
    TFarBox(TFarDialog *ADialog);

    // __property wstring Caption = { read = Data, write = Data };
    property<TFarBox, wstring> Caption;
    // __property bool Double = { read = GetAlterType, write = SetAlterType, index = DI_DOUBLEBOX };
    property<TFarBox, bool> Double;
};
//---------------------------------------------------------------------------
typedef void (*TFarButtonClick)(TFarButton *Sender, bool &Close);
enum TFarButtonBrackets { brNone, brTight, brSpace, brNormal };
//---------------------------------------------------------------------------
class TFarButton : public TFarDialogItem
{
public:
    TFarButton(TFarDialog *ADialog);

    // __property wstring Caption = { read = Data, write = Data };
    property<TFarButton, wstring> Caption;
    // __property int Result = { read = FResult, write = FResult };
    property<TFarButton, int> Result;
    // __property bool Default = { read = GetDefault, write = SetDefault };
    property<TFarButton, bool> Default;
    // __property TFarButtonBrackets Brackets = { read = FBrackets, write = SetBrackets };
    property<TFarButton, TFarButtonBrackets> Brackets;
    // __property CenterGroup;
    // property<TFarButton, bool> CenterGroup;
    // __property TFarButtonClick OnClick = { read = FOnClick, write = FOnClick };
    property<TFarButton, TFarButtonClick> OnClick;

protected:
    virtual void SetDataInternal(const wstring value);
    virtual wstring GetData();
    virtual long ItemProc(int Msg, long Param);
    virtual bool HotKey(char HotKey);

private:
    int FResult;
    TFarButtonClick FOnClick;
    TFarButtonBrackets FBrackets;

    void SetDefault(bool value);
    bool GetDefault();
    void SetBrackets(TFarButtonBrackets value);
};
//---------------------------------------------------------------------------
typedef void (*TFarAllowChange)(TFarDialogItem *Sender,
        long NewState, bool &AllowChange);
//---------------------------------------------------------------------------
class TFarCheckBox : public TFarDialogItem
{
public:
    TFarCheckBox(TFarDialog *ADialog);

    // __property wstring Caption = { read = Data, write = Data };
    property<TFarCheckBox, wstring> Caption;
    // __property bool AllowGrayed = { read = GetFlag, write = SetFlag, index = DIF_3STATE };
    property<TFarCheckBox, bool> AllowGrayed;
    // __property TFarAllowChange OnAllowChange = { read = FOnAllowChange, write = FOnAllowChange };
    property<TFarCheckBox, TFarAllowChange> OnAllowChange;
    // __property Checked;
    // __property Selected;

protected:
    TFarAllowChange FOnAllowChange;
    virtual long ItemProc(int Msg, long Param);
    virtual bool GetIsEmpty();
    virtual void SetData(const wstring value);
};
//---------------------------------------------------------------------------
class TFarRadioButton : public TFarDialogItem
{
public:
    TFarRadioButton(TFarDialog *ADialog);

    // __property Checked;
    // __property wstring Caption = { read = Data, write = Data };
    property<TFarRadioButton, wstring> Caption;
    // __property TFarAllowChange OnAllowChange = { read = FOnAllowChange, write = FOnAllowChange };
    property<TFarRadioButton, TFarAllowChange> OnAllowChange;

protected:
    TFarAllowChange FOnAllowChange;
    virtual long ItemProc(int Msg, long Param);
    virtual bool GetIsEmpty();
    virtual void SetData(const wstring value);
};
//---------------------------------------------------------------------------
class TFarEdit : public TFarDialogItem
{
public:
    TFarEdit(TFarDialog *ADialog);

    // __property wstring Text = { read = Data, write = Data };
    property<TFarEdit, wstring> Text;
    // __property int AsInteger = { read = GetAsInteger, write = SetAsInteger };
    property<TFarEdit, int> AsInteger;
    // __property bool Password = { read = GetAlterType, write = SetAlterType, index = DI_PSWEDIT };
    property<TFarEdit, bool> Password;
    // __property bool Fixed = { read = GetAlterType, write = SetAlterType, index = DI_FIXEDIT };
    property<TFarEdit, bool> Fixed;
    // __property wstring Mask = { read = GetHistoryMask, write = SetHistoryMask, index = 1 };
    property<TFarEdit, wstring> Mask;
    // __property wstring History = { read = GetHistoryMask, write = SetHistoryMask, index = 0 };
    property<TFarEdit, wstring> History;
    // __property bool ExpandEnvVars = { read = GetFlag, write = SetFlag, index = DIF_EDITEXPAND };
    property<TFarEdit, bool> ExpandEnvVars;
    // __property bool AutoSelect = { read = GetFlag, write = SetFlag, index = DIF_SELECTONENTRY };
    property<TFarEdit, bool> AutoSelect;
    // __property bool ReadOnly = { read = GetFlag, write = SetFlag, index = DIF_READONLY };
    property<TFarEdit, bool> ReadOnly;

protected:
    virtual long ItemProc(int Msg, long Param);
    virtual void Detach();

private:
    wstring GetHistoryMask(int Index);
    void SetHistoryMask(int Index, wstring value);
    void SetAsInteger(int value);
    int GetAsInteger();
};
//---------------------------------------------------------------------------
class TFarSeparator : public TFarDialogItem
{
public:
    TFarSeparator(TFarDialog *ADialog);

    // __property bool Double = { read = GetDouble, write = SetDouble };
    property<TFarSeparator, bool> Double;
    // __property wstring Caption = { read = Data, write = Data };
    property<TFarSeparator, wstring> Caption;
    // __property int Position = { read = GetPosition, write = SetPosition };
    property<TFarSeparator, int> Position;

protected:
    virtual void ResetBounds();

private:
    void SetDouble(bool value);
    bool GetDouble();
    void SetPosition(int value);
    int GetPosition();
};
//---------------------------------------------------------------------------
class TFarText : public TFarDialogItem
{
public:
    TFarText(TFarDialog *ADialog);

    // __property wstring Caption = { read = Data, write = Data };
    property<TFarText, wstring> Caption;
    // __property CenterGroup;
    // __property char Color = { read = GetColor, write = SetColor, index = 0 };
    property<TFarText, char> Color;

protected:
    virtual void SetData(const wstring value);
};
//---------------------------------------------------------------------------
class TFarListBox;
class TFarComboBox;
class TFarLister;
//---------------------------------------------------------------------------
class TFarList : public TStringList
{
    friend TFarListBox;
    friend TFarLister;
    friend TFarComboBox;
    typedef TFarList self;
public:
    TFarList(TFarDialogItem *ADialogItem = NULL);
    virtual ~TFarList();

    virtual void Assign(TPersistent *Source);

    // __property int Selected = { read = GetSelected, write = SetSelected };
    property<self, int> Selected;
    // __property int TopIndex = { read = GetTopIndex, write = SetTopIndex };
    property<self, int> TopIndex;
    // __property int MaxLength = { read = GetMaxLength };
    property<self, int> MaxLength;
    // __property int VisibleCount = { read = GetVisibleCount };
    property<self, int> VisibleCount;
    // __property unsigned int Flags[int Index] = { read = GetFlags, write = SetFlags };
    property<self, unsigned int> Flags;
    // __property bool Disabled[int Index] = { read = GetFlag, write = SetFlag, index = LIF_DISABLE };
    property<self, bool> Disabled;
    // __property bool Checked[int Index] = { read = GetFlag, write = SetFlag, index = LIF_CHECKED };
    property<self, bool> Checked;

protected:
    virtual void Changed();
    virtual long ItemProc(int Msg, long Param);
    virtual void Init();
    void UpdatePosition(int Position);
    int GetPosition();
    virtual void Put(int Index, const wstring S);
    void SetCurPos(int Position, int TopIndex);
    void UpdateItem(int Index);

    // __property FarList *ListItems = { read = FListItems };
    property<self, FarList *> ListItems;
    // __property TFarDialogItem *DialogItem = { read = FDialogItem };
    property<self, TFarDialogItem *> DialogItem;

private:
    FarList *FListItems;
    TFarDialogItem *FDialogItem;
    bool FNoDialogUpdate;

    inline int GetSelectedInt(bool Init);
    int GetSelected();
    void SetSelected(int value);
    int GetTopIndex();
    void SetTopIndex(int value);
    int GetMaxLength();
    int GetVisibleCount();
    unsigned int GetFlags(int Index);
    void SetFlags(int Index, unsigned int value);
    bool GetFlag(int Index, int Flag);
    void SetFlag(int Index, int Flag, bool value);
};
//---------------------------------------------------------------------------
enum TFarListBoxAutoSelect { asOnlyFocus, asAlways, asNever };
//---------------------------------------------------------------------------
class TFarListBox : public TFarDialogItem
{
    typedef TFarListBox self;
public:
    TFarListBox(TFarDialog *ADialog);
    ~TFarListBox();

    void SetItems(TStrings *value);

    // __property bool NoAmpersand = { read = GetFlag, write = SetFlag, index = DIF_LISTNOAMPERSAND };
    property<self, bool> NoAmpersand;
    // __property bool AutoHighlight = { read = GetFlag, write = SetFlag, index = DIF_LISTAUTOHIGHLIGHT };
    property<self, bool> AutoHighlight;
    // __property bool NoBox = { read = GetFlag, write = SetFlag, index = DIF_LISTNOBOX };
    property<self, bool> NoBox;
    // __property bool WrapMode = { read = GetFlag, write = SetFlag, index = DIF_LISTWRAPMODE };
    property<self, bool> WrapMode;
    // __property TFarList *Items = { read = FList, write = SetList };
    property<self, TFarList *> Items;
    // __property TFarListBoxAutoSelect AutoSelect = { read = FAutoSelect, write = SetAutoSelect };
    property<self, TFarListBoxAutoSelect> AutoSelect;

protected:
    virtual long ItemProc(int Msg, long Param);
    virtual void Init();
    virtual bool CloseQuery();

private:
    TFarList *FList;
    TFarListBoxAutoSelect FAutoSelect;
    bool FDenyClose;

    void SetAutoSelect(TFarListBoxAutoSelect value);
    void UpdateMouseReaction();
    void SetList(TFarList *value);
};
//---------------------------------------------------------------------------
class TFarComboBox : public TFarDialogItem
{
    typedef TFarComboBox self;
public:
    TFarComboBox(TFarDialog *ADialog);
    ~TFarComboBox();

    void ResizeToFitContent();

    // __property bool NoAmpersand = { read = GetFlag, write = SetFlag, index = DIF_LISTNOAMPERSAND };
    property<self, bool> NoAmpersand;
    // __property bool AutoHighlight = { read = GetFlag, write = SetFlag, index = DIF_LISTAUTOHIGHLIGHT };
    property<self, bool> AutoHighlight;
    // __property bool WrapMode = { read = GetFlag, write = SetFlag, index = DIF_LISTWRAPMODE };
    property<self, bool> WrapMode;
    // __property TFarList *Items = { read = FList };
    property<self, TFarList *> Items;
    // __property wstring Text = { read = Data, write = Data };
    property<self, wstring> Text;
    // __property bool AutoSelect = { read = GetFlag, write = SetFlag, index = DIF_SELECTONENTRY };
    property<self, bool> AutoSelect;
    // __property bool DropDownList = { read = GetFlag, write = SetFlag, index = DIF_DROPDOWNLIST };
    property<self, bool> DropDownList;

protected:
    virtual long ItemProc(int Msg, long Param);
    virtual void Init();

private:
    TFarList *FList;
};
//---------------------------------------------------------------------------
class TFarLister : public TFarDialogItem
{
    typedef TFarLister self;
public:
    TFarLister(TFarDialog *ADialog);
    virtual ~TFarLister();

    // __property TStrings *Items = { read = GetItems, write = SetItems };
    property<self, TStrings *> Items;
    // __property int TopIndex = { read = FTopIndex, write = SetTopIndex };
    property<self, int> TopIndex;
    // __property bool ScrollBar = { read = GetScrollBar };
    property<self, bool> ScrollBar;

protected:
    virtual long ItemProc(int Msg, long Param);
    virtual void DoFocus();

private:
    TStringList *FItems;
    int FTopIndex;

    void ItemsChange(TObject *Sender);
    bool GetScrollBar();
    TStrings *GetItems();
    void SetItems(TStrings *value);
    void SetTopIndex(int value);
};
//---------------------------------------------------------------------------
wstring StripHotKey(wstring Text);
TRect Rect(int Left, int Top, int Right, int Bottom);
//---------------------------------------------------------------------------
