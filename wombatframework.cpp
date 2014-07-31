#include "wombatframework.h"

WombatFramework::WombatFramework(WombatVariable* wombatvariable)
{
    wombatptr = wombatvariable;
}
WombatFramework::~WombatFramework()
{
}

void WombatFramework::OpenEvidenceImage() // open current evidence image
{
    const TSK_TCHAR** images;
    images = (const char**)malloc(wombatptr->evidenceobject.fullpathvector.size()*sizeof(char*));
    for(uint i=0; i < wombatptr->evidenceobject.fullpathvector.size(); i++)
    {
        images[i] = wombatptr->evidenceobject.fullpathvector[i].c_str();
    }
    wombatptr->evidenceobject.imageinfo = tsk_img_open(wombatptr->evidenceobject.itemcount, images, TSK_IMG_TYPE_DETECT, 0);
    if(wombatptr->evidenceobject.imageinfo == NULL)
    {
        LogEntry(wombatptr->caseobject.id, wombatptr->currentevidenceid, currentjobid, 0, "Evidence Image Access Failed.");
        errorcount++;
    }
    filesfound++;
    free(images);
}

void WombatFramework::OpenVolumeSystem() // open current volume system
{
    wombatptr->evidenceobject.volinfo = tsk_vs_open(wombatptr->evidenceobject.imageinfo, 0, TSK_VS_TYPE_DETECT);
    filesfound++;
}

void WombatFramework::GetVolumeSystemName() // get the volume system name
{
    if(wombatptr->evidenceobject.volinfo == NULL)
        wombatptr->currentvolumename = "Dummy Volume";
    else
        wombatptr->currentvolumename = QString::fromUtf8(tsk_vs_type_todesc(wombatptr->evidenceobject.volinfo->vstype));
}

void WombatFramework::GetFileSystemProperties() // get the file system label, boot sector, etc...
{
    for(uint i=0; i < wombatptr->evidenceobject.fsinfovector.size(); i++)
    {
        TSK_FS_INFO* tmpfsinfo = wombatptr->evidenceobject.fsinfovector[i];
        if(tmpfsinfo == NULL)
        {
            // log error here.
        }
        TSK_FS_FILE* tmpfile = NULL;
        //TSK_FS_DIR* tmpdir = NULL;
        //FATXXFS_SB* sb = NULL;
        FATXXFS_DENTRY* tmpfatdentry = NULL;
        FATXXFS_DENTRY* curentry = NULL;
        FATFS_INFO* fatfs = NULL;
        const TSK_FS_ATTR* tmpattr;
        TSK_DADDR_T cursector = 0;
        TSK_DADDR_T endsector = 0;
        int8_t isallocsec = 0;
        TSK_INUM_T curinum = 0;
        FATFS_DENTRY* dentry = NULL;
        char* databuffer = NULL;
        ssize_t cnt;
        ssize_t bytesread = 0;
        //int a;
        //uint8_t rval;
        char asc[512];
        switch(tmpfsinfo->ftype)
        {
            case TSK_FS_TYPE_NTFS:
                //NTFS_INFO* tmpinfo = (NTFS_INFO*)tmpfsinfo;
                if((tmpfile = tsk_fs_file_open_meta(tmpfsinfo, NULL, NTFS_MFT_VOL)) == NULL)
                {
                    // log error here...
                }
                tmpattr = tsk_fs_attrlist_get(tmpfile->meta->attr, TSK_FS_ATTR_TYPE_NTFS_VNAME);
                //tmpattr = tsk_fs_attrlist_get(tmpfile->meta->attr, NTFS_ATYPE_VNAME);
                if(!tmpattr)
                {
                    // log error here...
                }
                if((tmpattr->flags & TSK_FS_ATTR_RES) && (tmpattr->size))
                {
                    UTF16* name16 = (UTF16*) tmpattr->rd.buf;
                    UTF8* name8 = (UTF8*) asc;
                    int retval;
                    retval = tsk_UTF16toUTF8(tmpfsinfo->endian, (const UTF16**)&name16, (UTF16*) ((uintptr_t) name16 + (int) tmpattr->size), &name8, (UTF8*) ((uintptr_t)name8 + sizeof(asc)), TSKlenientConversion);
                    if(retval != TSKconversionOK)
                    {
                        // log error here                                
                        *name8 = '\0';
                    }
                    else if((uintptr_t) name8 >= (uintptr_t) asc + sizeof(asc))
                        asc[sizeof(asc) - 1] = '\0';
                    else
                        *name8 = '\0';
                }
                qDebug() << "Volume Name:" << asc;
                break;
            case TSK_FS_TYPE_EXFAT:
                fatfs = (FATFS_INFO*)tmpfsinfo;
                if((tmpfile = tsk_fs_file_alloc(tmpfsinfo)) == NULL)
                {
                    // log error here
                }
                if((tmpfile->meta = tsk_fs_meta_alloc(FATFS_FILE_CONTENT_LEN)) == NULL)
                {
                    // log error here
                }
                if((databuffer = (char*)tsk_malloc(fatfs->ssize)) == NULL)
                {
                    // log error here
                }
                cursector = fatfs->rootsect;
                endsector = fatfs->firstdatasect + (fatfs->clustcnt * fatfs->csize) - 1;
                while(cursector < endsector)
                {

                }
                bytesread = tsk_fs_read_block(tmpfsinfo, cursector, databuffer, fatfs->ssize);
                if(bytesread != fatfs->ssize)
                {
                    // log error here
                }
                isallocsec = fatfs_is_sectalloc(fatfs, cursector);
                if(isallocsec == -1)
                {
                    // log error here
                }
                curinum = FATFS_SECT_2_INODE(fatfs, cursector);
                for(i = 0; i < fatfs->ssize; i+= sizeof(FATFS_DENTRY))
                {
                    dentry = (FATFS_DENTRY*)&(databuffer[i]);
                    if(exfatfs_get_enum_from_type(dentry->data[0]) == EXFATFS_DIR_ENTRY_TYPE_VOLUME_LABEL)
                    {
                        if(exfatfs_dinode_copy(fatfs, curinum, dentry, isallocsec, tmpfile) == TSK_OK)
                        {
                            qDebug() << "EXFAT Volume Name: " << tmpfile->meta->name2->name;
                            break;
                        }
                        else
                        {
                            // log error here
                        }
                    }
                }
                break;
            case TSK_FS_TYPE_FAT12:
                fatfs = (FATFS_INFO*)tmpfsinfo;
                if((databuffer = (char*) tsk_malloc(tmpfsinfo->block_size)) == NULL)
                {
                    // log error here
                }
                cnt = tsk_fs_read_block(tmpfsinfo, fatfs->rootsect, databuffer, tmpfsinfo->block_size);
                if(cnt != tmpfsinfo->block_size)
                {
                    // log error here
                }
                tmpfatdentry = NULL;
                if(fatfs->ssize <= tmpfsinfo->block_size)
                {
                    curentry = (FATXXFS_DENTRY*)databuffer;
                    for(int i=0; i < fatfs->ssize; i += sizeof(*curentry))
                    {
                        if(curentry->attrib == FATFS_ATTR_VOLUME)
                        {
                            tmpfatdentry = curentry;
                            break;
                        }
                        curentry++;
                    }
                }
                qDebug() << "FAT12 Volume Label: " << tmpfatdentry->name;
                qDebug() << "FAT12";
                break;
            case TSK_FS_TYPE_FAT16:
                fatfs = (FATFS_INFO*)tmpfsinfo;
                if((databuffer = (char*) tsk_malloc(tmpfsinfo->block_size)) == NULL)
                {
                    // log error here
                }
                cnt = tsk_fs_read_block(tmpfsinfo, fatfs->rootsect, databuffer, tmpfsinfo->block_size);
                if(cnt != tmpfsinfo->block_size)
                {
                    // log error here
                }
                tmpfatdentry = NULL;
                if(fatfs->ssize <= tmpfsinfo->block_size)
                {
                    curentry = (FATXXFS_DENTRY*)databuffer;
                    for(int i=0; i < fatfs->ssize; i += sizeof(*curentry))
                    {
                        if(curentry->attrib == FATFS_ATTR_VOLUME)
                        {
                            tmpfatdentry = curentry;
                            break;
                        }
                        curentry++;
                    }
                }
                qDebug() << "FAT16 Volume Label: " << tmpfatdentry->name;
                //qDebug() << "FAT16";
                break;
            case TSK_FS_TYPE_FAT32:
                fatfs = (FATFS_INFO*)tmpfsinfo;
                if((databuffer = (char*) tsk_malloc(tmpfsinfo->block_size)) == NULL)
                {
                    // log error here
                }
                cnt = tsk_fs_read_block(tmpfsinfo, fatfs->rootsect, databuffer, tmpfsinfo->block_size);
                if(cnt != tmpfsinfo->block_size)
                {
                    // log error here
                }
                tmpfatdentry = NULL;
                if(fatfs->ssize <= tmpfsinfo->block_size)
                {
                    curentry = (FATXXFS_DENTRY*)databuffer;
                    for(int i=0; i < fatfs->ssize; i += sizeof(*curentry))
                    {
                        if(curentry->attrib == FATFS_ATTR_VOLUME)
                        {
                            tmpfatdentry = curentry;
                            break;
                        }
                        curentry++;
                    }
                }
                qDebug() << "FAT32 Volume Label: " << tmpfatdentry->name;
                //qDebug() << "FAT32";
                break;
            case TSK_FS_TYPE_FFS1:
                qDebug() << "FFS1";
                break;
            case TSK_FS_TYPE_FFS1B:
                qDebug() << "FFS1B";
                break;
            case TSK_FS_TYPE_FFS2:
                qDebug() << "FFS2";
                break;
            case TSK_FS_TYPE_EXT2:
                qDebug() << "EXT2";
                break;
            case TSK_FS_TYPE_EXT3:
                qDebug() << "EXT3";
                break;
            case TSK_FS_TYPE_EXT4:
                qDebug() << "EXT4";
                break;
            case TSK_FS_TYPE_RAW:
                qDebug() << "no file system. store 0, \"\", or message for respective variables";
                break;
            case TSK_FS_TYPE_ISO9660:
                qDebug() << "ISO9660";
                break;
            case TSK_FS_TYPE_HFS:
                qDebug() << "HFS";
                break;
            case TSK_FS_TYPE_YAFFS2:
                qDebug() << "YAFFS2";
                break;
            case TSK_FS_TYPE_SWAP:
                qDebug() << "no file system. store 0, \"\", or message for respective variables";
                break;
           default:
                qDebug() << "what to do for default???";
                break;
        }
    }
}

void WombatFramework::OpenPartitions() // open the partitions in the volume
{
    wombatptr->evidenceobject.fsinfovector.clear();
    // CHECK IF VOLUME EXISTS, IF NOT THEN SHOULD GET FS FROM IMG INSTEAD OF VOL.
    if(wombatptr->evidenceobject.volinfo == NULL)
    {
        wombatptr->evidenceobject.fsinfovector.push_back(tsk_fs_open_img(wombatptr->evidenceobject.imageinfo, 0, TSK_FS_TYPE_DETECT));
        filesfound++;
    }
    else
    {
        if(wombatptr->evidenceobject.volinfo->part_count > 0)
        {
            for(uint32_t i=0; i < wombatptr->evidenceobject.volinfo->part_count; i++)
            {
                if(tsk_vs_part_get(wombatptr->evidenceobject.volinfo, i)->flags == 0x02) // if its an unallocated partition
                {
                    filesfound++;
                }
                wombatptr->evidenceobject.partinfovector.push_back(tsk_vs_part_get(wombatptr->evidenceobject.volinfo, i));
                /*
                if(tsk_vs_part_get(wombatptr->evidenceobject.volinfo, i)->flags == 0x01) // contains an allocated file system.
                {
                    TSK_FS_INFO* tmpfsinfo = tsk_fs_open_vol(wombatptr->evidenceobject.partinfovector[i], TSK_FS_TYPE_DETECT);
                    if(tmpfsinfo != NULL)
                    {
                        wombatptr->evidenceobject.fsinfovector.push_back(tmpfsinfo);
                        filesfound++;
                    }
                    else
                    {
                        LogEntry(wombatptr->caseobject.id, wombatptr->currentevidenceid, currentjobid, 0, "Failed to Open Partition/FileSystem");
                        errorcount++;
                    }
                }*/
            }
        }
    }
}

void WombatFramework::OpenFiles() // open the files and add to file info vector
{
    uint8_t walkreturn;
    int walkflags = TSK_FS_DIR_WALK_FLAG_ALLOC | TSK_FS_DIR_WALK_FLAG_UNALLOC | TSK_FS_DIR_WALK_FLAG_RECURSE;
    for(uint i=0; i < wombatptr->evidenceobject.fsinfovector.size(); i++)
    {
        currentfilesystemid = wombatptr->evidenceobject.fsidvector[i];
        if(fcasedb.transaction())
        {
            walkreturn = tsk_fs_dir_walk(wombatptr->evidenceobject.fsinfovector[i], wombatptr->evidenceobject.fsinfovector[i]->root_inum, (TSK_FS_DIR_WALK_FLAG_ENUM)walkflags, FileEntries, NULL);
        }
    }
    if(walkreturn == 1)
    {
        LogEntry(wombatptr->caseobject.id, wombatptr->currentevidenceid, currentjobid, 0, "Issues with Traversing the File Structure were encountered.");
        errorcount++;
    }
}

void WombatFramework::CloseInfoStructures() // close all open info structures
{
    for(int j=0; j < wombatptr->evidenceobjectvector.count(); j++)
    {
        for(uint i=0; i < wombatptr->evidenceobjectvector[j].fsinfovector.size(); i++)
        {
            if(wombatptr->evidenceobjectvector[j].fsinfovector[i] != NULL)
            {
                tsk_fs_close(wombatptr->evidenceobjectvector[j].fsinfovector[i]);
                wombatptr->evidenceobjectvector[j].fsinfovector[i] = NULL;
            }
        }
        wombatptr->evidenceobjectvector[j].fsinfovector.clear();
        for(uint i=0; i < wombatptr->evidenceobject.partinfovector.size(); i++)
        {
            wombatptr->evidenceobjectvector[j].partinfovector[i] = NULL;
        }
        wombatptr->evidenceobjectvector[j].partinfovector.clear();
        if(wombatptr->evidenceobjectvector[j].volinfo != NULL)
        {
            tsk_vs_close(wombatptr->evidenceobjectvector[j].volinfo);
            wombatptr->evidenceobjectvector[j].volinfo = NULL;
        }
        if(wombatptr->evidenceobjectvector[j].imageinfo != NULL)
        {
            tsk_img_close(wombatptr->evidenceobjectvector[j].imageinfo);
            wombatptr->evidenceobjectvector[j].imageinfo = NULL;
        }
    }
}
                // BEGIN ATTRIBUTE LOOP TEST
                /*
                TSK_FS_FILE* rootfileinfo = tsk_fs_file_open_meta(tmpfsinfo, NULL, tmpfsinfo->root_inum);
                if(tsk_fs_file_attr_getsize(rootfileinfo) > 0)
                {
                    for(int i=0; i < tsk_fs_file_attr_getsize(rootfileinfo); i++)
                    {
                        TSK_FS_ATTR* tmpattr;
                        if(i == 0)
                            tmpattr = rootfileinfo->meta->attr->head;
                        else
                            tmpattr = rootfileinfo->meta->attr->head->next;
                        if(tmpattr != NULL)
                        {
                            if(tmpattr->name != NULL)
                                qDebug() << QString("Attribute ID|Name is: ") << QString::number(tmpattr->id) << "|" << QString(tmpattr->name);
                            else
                                qDebug() << "Attribute Name is null ID is: " << QString::number(tmpattr->id);
                        }
                        else
                        {
                            qDebug() << "attribute is null";
                        }
                    }
                }
                else
                {
                    qDebug() << "root inum has no attributes.";
                }*/
                // END ATTRIBUTE LOOP TEST

/*
void WombatFramework::GetBootCode(int idx) // deermine boot type and populate variable if exists otherwise populate wiht negative
{
    //qDebug() << "Might not need " << idx;
    int volidx = -1;
    for(int i=0; i < wombatptr->volumeobjectvector.count(); i++)
    {
        if(wombatptr->selectedobject.id == wombatptr->volumeobjectvector[i].parentid) // boot volume exists
            volidx = i;
    }
    if(volidx > -1) // boot volume exists
    {
        wombatptr->htmlcontent += "<tr><td class='property'>byte offset</td><td class='pvalue'>";
        wombatptr->htmlcontent += QLocale::system().toString((int)wombatptr->volumeobjectvector[volidx].byteoffset) + "</td></tr>";
        wombatptr->htmlcontent += "<tr><td class='property'>block size</td><td class='pvalue'>";
        wombatptr->htmlcontent += QLocale::system().toString((int)wombatptr->volumeobjectvector[volidx].blocksize) + " bytes</td></tr>";
        wombatptr->htmlcontent += "<tr><td class='property'>endian ordering</td><td class='pvalue'>";
        if(wombatptr->evidenceobject.volinfo->endian == TSK_LIT_ENDIAN)
            wombatptr->htmlcontent += "little endian";
        else
            wombatptr->htmlcontent += "big endian";
        wombatptr->htmlcontent += "</td></tr>";
        wombatptr->htmlcontent += "<tr><td class='property'>partition count</td><td class='pvalue'>";
        wombatptr->htmlcontent += QLocale::system().toString((int)wombatptr->volumeobjectvector[volidx].childcount) + "</td></tr></table>";
        //wombatptr->htmlcontent += "<br/><br/><div class='tabletitle'>boot sector</div>";
        //wombatptr->htmlcontent += "<br/><table><tr><th>byte offset</th><th>value</th><th>description</th></tr>";
        //wombatptr->htmlcontent += "<tr class='odd'><td>0-0</td><td class='bvalue'></td><td class='desc'></td></tr>";
        //wombatptr->htmlcontent += "<tr class='odd'><td colspan='3' class='bot'></td></tr></table>";
        // populate the boot sector table
        // PARTITION INFORMATION I NEED TO RECORD: ACTIVE/BOOTABLE, PARTITION TYPE, LBA OF FIRST ABSOLUTE SECTOR OF PARTITION,# OF SECTORS IN PARTITION
        // PARTITION INFORMATION FROM TSK INFO   : FLAGS, LEN (# OF SECTORS), START, (FIRST SECTOR), DESC, SLOT_NUM, TABLE_NUM
    }
    else // not a bootable volume
    {
        wombatptr->htmlcontent += "</table><br/><br/><div class='tabletitle'>not a bootable volume</div>";
        // block size, partition count, offset where volume system begins, endian - probably go in a table similar to the above one, not a byte offset one.
        // not sure if i want to include the minimal amount of sector info since there are so few and it'll be repetative with the possible partition
        // layout in the tree view
    }*/
//}
/*
int WombatFramework::DetermineVectorIndex()
{*/
    /*
    // the objectvectors and objects were replaced. i need to get it straight from the sql call...
    // so i shouldn't need this function or the currentidx since i can just get the sql call with the id.
    int curidx = -1;
    if(wombatptr->selectedobject.type == 1) // image file
    {
        for(int i=0; i < wombatptr->evidenceobjectvector.count(); i++)
        {
            if(wombatptr->evidenceobjectvector[i].id == wombatptr->selectedobject.id)
                curidx = i;
        }
    }
    else if(wombatptr->selectedobject.type == 2) // volume file (it should never be a volume since i don't add it to the image tree)
    {
        for(int i=0; i < wombatptr->volumeobjectvector.count(); i++)
        {
            if(wombatptr->volumeobjectvector[i].id == wombatptr->selectedobject.id)
                curidx = i;
        }
    }
    else if(wombatptr->selectedobject.type == 3) // partition file
    {
        for(int i=0; i < wombatptr->partitionobjectvector.count(); i++)
        {
            if(wombatptr->partitionobjectvector[i].id == wombatptr->selectedobject.id)
                curidx = i;
        }
    }
    else if(wombatptr->selectedobject.type == 4) // file system file
    {
        for(int i=0; i < wombatptr->filesystemobjectvector.count(); i++)
        {
            if(wombatptr->filesystemobjectvector[i].id == wombatptr->selectedobject.id)
                curidx = i;
        }
    }
    else // implement for files, directories etc.. as i go.
    {
    }

    return curidx;
    */
/*
    return 0;
}*/
