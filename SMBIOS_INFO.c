#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>
#include <Library/FileHandleLib.h>
#include <Library/DevicePathLib.h>
#include <Library/BaseMemoryLib.h> 
#include <Guid/SmBios.h>
#include <Protocol/Smbios.h>
#include <Library/DebugLib.h>

 EFI_GUID gEfiSmbiosProtocolGuid = EFI_SMBIOS_PROTOCOL_GUID;


EFI_STATUS EFIAPI UefiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
  EFI_STATUS Status;
  EFI_MEMORY_DESCRIPTOR *MemoryMap = NULL;
  UINTN MemoryMapSize = 0;
  UINTN MapKey;
  UINTN DescriptorSize;
  UINT32 DescriptorVersion;
  UINTN Index;
  UINT64 TotalRAMSize = 0;
  
  // Print the RAM size in bytes
  Print(L"RAM Size: %lu GB\n");
  gBS->Stall(5000000);

 // Retrieve the memory map
  MemoryMapSize = 0;
  Status = gBS->GetMemoryMap(&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    MemoryMapSize += 2 * DescriptorSize; // Increase the buffer size to avoid truncation

    // Allocate memory for the memory map
    Status = gBS->AllocatePool(EfiLoaderData, MemoryMapSize, (void**)&MemoryMap);
    if (EFI_ERROR(Status)) {
      Print(L"Failed to allocate memory for the memory map.\n");
gBS->Stall(5000000);
      return Status;
    }

    // Retrieve the updated memory map
    Status = gBS->GetMemoryMap(&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (EFI_ERROR(Status)) {
      Print(L"Failed to retrieve the memory map.\n");
gBS->Stall(5000000);
      gBS->FreePool(MemoryMap);
      return Status;
    }

// Iterate through the memory map entries
    for (Index = 0; Index < MemoryMapSize / DescriptorSize; Index++) {
      EFI_MEMORY_DESCRIPTOR *Descriptor = (EFI_MEMORY_DESCRIPTOR*)((UINTN)MemoryMap + Index * DescriptorSize);
      if (Descriptor->Type == EfiConventionalMemory) {
        TotalRAMSize += Descriptor->NumberOfPages * EFI_PAGE_SIZE;
      }
    }

    // Free the memory allocated for the memory map
    Status = gBS->FreePool(MemoryMap);
    if (EFI_ERROR(Status)) {
      Print(L"Failed to free the memory map.\n");
      return Status;
    }

    // Convert RAM size to GB (gigabytes)
    UINT64 TotalRAMSizeGB = TotalRAMSize / (1024 * 1024 * 1024);

    // Print the RAM size in GB
    Print(L"RAM Size: %lu GB\n", TotalRAMSizeGB);
  } else {
    Print(L"Failed to retrieve the memory map size.\n");
    return Status;
  }

    UINT64 TotalRAMSizeGB = TotalRAMSize / (1024 * 1024 * 1024);
    // Print the RAM size in bytes
    Print(L"RAM Size: %lu GB\n", TotalRAMSizeGB);
   
    gBS->Stall(5000000);
    
    
    
    
    
   
   
   EFI_SMBIOS_PROTOCOL *SmbiosProtocol;

  // Locate the SMBIOS protocol
  Status = gBS->LocateProtocol(&gEfiSmbiosProtocolGuid, NULL, (VOID **)&SmbiosProtocol);
  if (EFI_ERROR(Status)) {
    Print(L"Failed to locate the SMBIOS protocol.\n");
    gBS->Stall(5000000);
    return Status;
  }

  // Get the SMBIOS table handle
  EFI_SMBIOS_HANDLE SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  EFI_SMBIOS_TYPE SmbiosType = EFI_SMBIOS_TYPE_BIOS_INFORMATION;

  while (TRUE) {
    EFI_SMBIOS_TABLE_HEADER *SmbiosRecord;

    // Retrieve the next SMBIOS record
    Status = SmbiosProtocol->GetNext(SmbiosProtocol, &SmbiosHandle, &SmbiosType, &SmbiosRecord, NULL);
    if (EFI_ERROR(Status)) {
      if (Status == EFI_NOT_FOUND) {
        // Reached the end of SMBIOS tables
        break;
      } else {
        Print(L"Failed to retrieve the next SMBIOS record.\n");
        return Status;
      }
    }

    // Process the BIOS Information record (Type 0)
    if (SmbiosRecord->Type == EFI_SMBIOS_TYPE_BIOS_INFORMATION) {
     // EFI_SMBIOS_TYPE_BIOS_INFORMATION *BiosInfo = (EFI_SMBIOS_TYPE_BIOS_INFORMATION *)SmbiosRecord;

      // Calculate the string offsets within the record
      CHAR8 *Vendor = (CHAR8 *)SmbiosRecord + SmbiosRecord->Length;
      CHAR8 *Version = Vendor + AsciiStrLen(Vendor) + 1;
      CHAR8 *ReleaseDate = Version + AsciiStrLen(Version) + 1;

      // Print the BIOS information
      Print(L"BIOS Vendor: %a\n", Vendor);
      Print(L"BIOS Version: %a\n", Version);
      Print(L"BIOS Release Date: %a\n", ReleaseDate);

      // Print the BIOS content and its ASCII characters
      Print(L"BIOS Content:\n");
      UINTN BiosContentLength = SmbiosRecord->Length;
      UINT8 *BiosContent = (UINT8 *)SmbiosRecord;
      for (UINTN i = 0; i < BiosContentLength; i++) {
        if (i % 16 == 0) {
          Print(L"\n");
        }
        Print(L"%02x ", BiosContent[i]);
      }
      Print(L"\n");

      // Print the BIOS content as ASCII characters
      Print(L"BIOS Content (ASCII):\n");
      for (UINTN i = 0; i < BiosContentLength; i++) {
        if (i % 16 == 0) {
          Print(L"\n");
        }
        if (BiosContent[i] >= 0x20 && BiosContent[i] <= 0x7E) {
          Print(L"%c ", BiosContent[i]);
        } else {
          Print(L". ");
        }
      }
      Print(L"\n");

      break;  // Exit the loop after processing the BIOS Information record
    }

    // Move to the next record
    SmbiosHandle = SmbiosRecord->Handle + 1;
  }

  gBS->Stall(10000000);

  return EFI_SUCCESS;
}
