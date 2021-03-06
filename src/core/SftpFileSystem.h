
#pragma once

#include <stdint.h>
#include <FileSystems.h>

class TSFTPPacket;
struct TOverwriteFileParams;
struct TSFTPSupport;
class TSecureShell;

//enum TSFTPOverwriteMode { omOverwrite, omAppend, omResume };
extern const intptr_t SFTPMaxVersion;

class TSFTPFileSystem : public TCustomFileSystem
{
NB_DISABLE_COPY(TSFTPFileSystem)
friend class TSFTPPacket;
friend class TSFTPQueue;
friend class TSFTPAsynchronousQueue;
friend class TSFTPUploadQueue;
friend class TSFTPDownloadQueue;
friend class TSFTPLoadFilesPropertiesQueue;
friend class TSFTPCalculateFilesChecksumQueue;
friend class TSFTPBusy;
public:
  explicit TSFTPFileSystem(TTerminal * ATermina);
  virtual ~TSFTPFileSystem();

  virtual void Init(void * Data); // TSecureShell *
  virtual void FileTransferProgress(int64_t /*TransferSize*/, int64_t /*Bytes*/) {}

  virtual void Open();
  virtual void Close();
  virtual bool GetActive() const;
  virtual void CollectUsage();
  virtual void Idle();
  virtual UnicodeString GetAbsolutePath(const UnicodeString & APath, bool Local);
  virtual UnicodeString GetAbsolutePath(const UnicodeString & APath, bool Local) const;
  virtual void AnyCommand(const UnicodeString & Command,
    TCaptureOutputEvent OutputEvent);
  virtual void ChangeDirectory(const UnicodeString & Directory);
  virtual void CachedChangeDirectory(const UnicodeString & Directory);
  virtual void AnnounceFileListOperation();
  virtual void ChangeFileProperties(const UnicodeString & AFileName,
    const TRemoteFile * AFile, const TRemoteProperties * Properties,
    TChmodSessionAction & Action);
  virtual bool LoadFilesProperties(TStrings * AFileList);
  virtual void CalculateFilesChecksum(const UnicodeString & Alg,
    TStrings * AFileList, TStrings * Checksums,
    TCalculatedChecksumEvent OnCalculatedChecksum);
  virtual void CopyToLocal(const TStrings * AFilesToCopy,
    const UnicodeString & TargetDir, const TCopyParamType * CopyParam,
    intptr_t Params, TFileOperationProgressType * OperationProgress,
    TOnceDoneOperation & OnceDoneOperation);
  virtual void CopyToRemote(const TStrings * AFilesToCopy,
    const UnicodeString & TargetDir, const TCopyParamType * CopyParam,
    intptr_t Params, TFileOperationProgressType * OperationProgress,
    TOnceDoneOperation & OnceDoneOperation);
  virtual void RemoteCreateDirectory(const UnicodeString & ADirName);
  virtual void CreateLink(const UnicodeString & AFileName, const UnicodeString & PointTo, bool Symbolic);
  virtual void RemoteDeleteFile(const UnicodeString & AFileName,
    const TRemoteFile * AFile, intptr_t Params, TRmSessionAction & Action);
  virtual void CustomCommandOnFile(const UnicodeString & AFileName,
    const TRemoteFile * AFile, const UnicodeString & Command, intptr_t Params, TCaptureOutputEvent OutputEvent);
  virtual void DoStartup();
  virtual void HomeDirectory();
  virtual bool IsCapable(intptr_t Capability) const;
  virtual void LookupUsersGroups();
  virtual void ReadCurrentDirectory();
  virtual void ReadDirectory(TRemoteFileList * FileList);
  virtual void ReadFile(const UnicodeString & AFileName,
    TRemoteFile *& AFile);
  virtual void ReadSymlink(TRemoteFile * SymlinkFile,
    TRemoteFile *& AFile);
  virtual void RemoteRenameFile(const UnicodeString & AFileName,
    const UnicodeString & NewName);
  virtual void RemoteCopyFile(const UnicodeString & AFileName,
    const UnicodeString & ANewName);
  virtual TStrings * GetFixedPaths();
  virtual void SpaceAvailable(const UnicodeString & APath,
    TSpaceAvailable & ASpaceAvailable);
  virtual const TSessionInfo & GetSessionInfo() const;
  virtual const TFileSystemInfo & GetFileSystemInfo(bool Retrieve);
  virtual bool TemporaryTransferFile(const UnicodeString & AFileName);
  virtual bool GetStoredCredentialsTried() const;
  virtual UnicodeString FSGetUserName() const;
  virtual void GetSupportedChecksumAlgs(TStrings * Algs);

protected:
  TSecureShell * FSecureShell;
  TFileSystemInfo FFileSystemInfo;
  bool FFileSystemInfoValid;
  intptr_t FVersion;
  UnicodeString FCurrentDirectory;
  UnicodeString FDirectoryToChangeTo;
  UnicodeString FHomeDirectory;
  AnsiString FEOL;
  TList * FPacketReservations;
  rde::vector<uintptr_t> FPacketNumbers;
  uint8_t FPreviousLoggedPacket;
  int FNotLoggedPackets;
  int FBusy;
  void * FBusyToken;
  bool FAvoidBusy;
  TStrings * FExtensions;
  TSFTPSupport * FSupport;
  TAutoSwitch FUtfStrings;
  bool FUtfDisablingAnnounced;
  bool FSignedTS;
  TStrings * FFixedPaths;
  uint32_t FMaxPacketSize;
  bool FSupportsStatVfsV2;
  uintptr_t FCodePage;
  bool FSupportsHardlink;
  std::unique_ptr<TStringList> FChecksumAlgs;
  std::unique_ptr<TStringList> FChecksumSftpAlgs;

  void SendCustomReadFile(TSFTPPacket * Packet, TSFTPPacket * Response,
    uint32_t Flags);
  void CustomReadFile(const UnicodeString & AFileName,
    TRemoteFile *& AFile, uint8_t Type, TRemoteFile * ALinkedByFile = nullptr,
    intptr_t AllowStatus = -1);
  virtual UnicodeString GetCurrDirectory() const;
  UnicodeString GetHomeDirectory();
  uintptr_t GotStatusPacket(TSFTPPacket * Packet, intptr_t AllowStatus);
  bool RemoteFileExists(const UnicodeString & FullPath, TRemoteFile ** AFile = nullptr);
  TRemoteFile * LoadFile(TSFTPPacket * Packet,
    TRemoteFile * ALinkedByFile, const UnicodeString & AFileName,
    TRemoteFileList * TempFileList = nullptr, bool Complete = true);
  void LoadFile(TRemoteFile * AFile, TSFTPPacket * Packet,
    bool Complete = true);
  UnicodeString LocalCanonify(const UnicodeString & APath) const;
  UnicodeString Canonify(const UnicodeString & APath);
  UnicodeString GetRealPath(const UnicodeString & APath);
  UnicodeString GetRealPath(const UnicodeString & APath, const UnicodeString & ABaseDir);
  void ReserveResponse(const TSFTPPacket * Packet,
    TSFTPPacket * Response);
  uintptr_t ReceivePacket(TSFTPPacket * Packet, intptr_t ExpectedType = -1,
    intptr_t AllowStatus = -1);
  bool PeekPacket();
  void RemoveReservation(intptr_t Reservation);
  void SendPacket(const TSFTPPacket * Packet);
  uintptr_t ReceiveResponse(const TSFTPPacket * Packet,
    TSFTPPacket * AResponse, intptr_t ExpectedType = -1, intptr_t AllowStatus = -1);
  uintptr_t SendPacketAndReceiveResponse(const TSFTPPacket * Packet,
    TSFTPPacket * Response, intptr_t ExpectedType = -1, intptr_t AllowStatus = -1);
  void UnreserveResponse(TSFTPPacket * Response);
  void TryOpenDirectory(const UnicodeString & Directory);
  bool SupportsExtension(const UnicodeString & Extension) const;
  void ResetConnection();
  void DoCalculateFilesChecksum(
    const UnicodeString & Alg, const UnicodeString & SftpAlg,
    TStrings * AFileList, TStrings * Checksums,
    TCalculatedChecksumEvent OnCalculatedChecksum,
    TFileOperationProgressType * OperationProgress, bool FirstLevel);
  void RegisterChecksumAlg(const UnicodeString & Alg, const UnicodeString & SftpAlg);
  void DoDeleteFile(const UnicodeString & AFileName, uint8_t Type);

  void SFTPSourceRobust(const UnicodeString & AFileName,
    const TRemoteFile * AFile,
    const UnicodeString & TargetDir, const TCopyParamType * CopyParam, intptr_t Params,
    TFileOperationProgressType * OperationProgress, uintptr_t Flags);
  void SFTPSource(const UnicodeString & AFileName,
    const TRemoteFile * AFile,
    const UnicodeString & TargetDir, const TCopyParamType * CopyParam, intptr_t Params,
    TOpenRemoteFileParams & OpenParams,
    TOverwriteFileParams & FileParams,
    TFileOperationProgressType * OperationProgress, uintptr_t Flags,
    TUploadSessionAction & Action, bool & ChildError);
  RawByteString SFTPOpenRemoteFile(const UnicodeString & AFileName,
    uint32_t OpenType, int64_t Size = -1);
  intptr_t SFTPOpenRemote(void * AOpenParams, void * Param2);
  void SFTPCloseRemote(const RawByteString & Handle,
    const UnicodeString & AFileName, TFileOperationProgressType * OperationProgress,
    bool TransferFinished, bool Request, TSFTPPacket * Packet);
  void SFTPDirectorySource(const UnicodeString & DirectoryName,
    const UnicodeString & TargetDir, uintptr_t LocalFileAttrs, const TCopyParamType * CopyParam,
    intptr_t Params, TFileOperationProgressType * OperationProgress, uintptr_t Flags);
  void SFTPConfirmOverwrite(const UnicodeString & AFullFileName, UnicodeString & AFileName,
    const TCopyParamType * CopyParam, intptr_t Params, TFileOperationProgressType * OperationProgress,
    const TOverwriteFileParams * FileParams,
    OUT TOverwriteMode & Mode);
  bool SFTPConfirmResume(const UnicodeString & DestFileName, bool PartialBiggerThanSource,
    TFileOperationProgressType * OperationProgress);
  void SFTPSinkRobust(const UnicodeString & AFileName,
    const TRemoteFile * AFile, const UnicodeString & TargetDir,
    const TCopyParamType * CopyParam, intptr_t Params,
    TFileOperationProgressType * OperationProgress, uintptr_t Flags);
  void SFTPSink(const UnicodeString & AFileName,
    const TRemoteFile * AFile, const UnicodeString & TargetDir,
    const TCopyParamType * CopyParam, intptr_t Params,
    TFileOperationProgressType * OperationProgress, uintptr_t Flags,
    TDownloadSessionAction & Action, bool & ChildError);
  void SFTPSinkFile(const UnicodeString & AFileName,
    const TRemoteFile * AFile, void * Param);
  char * GetEOL() const;
  inline void BusyStart();
  inline void BusyEnd();
  uint32_t TransferBlockSize(uint32_t Overhead,
    TFileOperationProgressType * OperationProgress,
    uint32_t MinPacketSize = 0,
    uint32_t MaxPacketSize = 0);
  uint32_t UploadBlockSize(const RawByteString & Handle,
    TFileOperationProgressType * OperationProgress);
  uint32_t DownloadBlockSize(
    TFileOperationProgressType * OperationProgress);
  intptr_t PacketLength(uint8_t * LenBuf, intptr_t ExpectedType);

private:
  const TSessionData * GetSessionData() const;
};

