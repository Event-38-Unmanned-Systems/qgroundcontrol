/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "QGCZip.h"

#include <QFile>
#include <QDir>
#include <QtDebug>


int QGCZip::do_extract(unzFile uf,int opt_extract_without_path,int opt_overwrite,const char* filepath){
    uLong i;
        unz_global_info64 gi;
        int err;

        err = unzGetGlobalInfo64(uf,&gi);
        if (err!=UNZ_OK)
            printf("error %d with zipfile in unzGetGlobalInfo \n",err);

        for (i=0;i<gi.number_entry;i++)
        {
            if (do_extract_currentfile(uf,&opt_extract_without_path,
                                          &opt_overwrite,filepath) != UNZ_OK)
                break;

            if ((i+1)<gi.number_entry)
            {
                err = unzGoToNextFile(uf);
                if (err!=UNZ_OK)
                {
                    printf("error %d with zipfile in unzGoToNextFile\n",err);
                    break;
                }
            }
        }

        return 0;

}

int QGCZip::do_extract_currentfile(unzFile uf,const int* popt_extract_without_path,int* popt_overwrite, const char* filepath){
    char filename_inzip[256];
    char* filename_withoutpath;
    char* p;
    int err=UNZ_OK;
    FILE *fout=NULL;
    void* buf;
    uInt size_buf;

    unz_file_info64 file_info;
    err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);

    if (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        return err;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (void*)malloc(size_buf);
    if (buf==NULL)
    {
        printf("Error allocating memory\n");
        return UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    while ((*p) != '\0')
    {
        if (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }


        char* write_filename;
        int skip=0;



            write_filename = new char[strlen(filepath)+strlen(filename_inzip)+1];
            strcpy(write_filename,filepath);
            strcat(write_filename,filename_inzip);

        err = unzOpenCurrentFilePassword(uf,NULL);
        if (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        if (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            char rep=0;
            FILE* ftestexist;
            ftestexist = fopen(write_filename,"rb");
            if (ftestexist!=NULL)
            {
                fclose(ftestexist);
                do
                {
                    char answer[128];
                    int ret;

                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
                    ret = scanf("%1s",answer);
                    if (ret != 1)
                    {
                       exit(EXIT_FAILURE);
                    }
                    rep = answer[0] ;
                    if ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                while ((rep!='Y') && (rep!='N') && (rep!='A'));
            }

            if (rep == 'N')
                skip = 1;

            if (rep == 'A')
                *popt_overwrite=1;
        }

        if ((skip==0) && (err==UNZ_OK))
        {
            fout=fopen(write_filename,"wb");
        }

        if (fout!=NULL)
        {
            printf(" extracting: %s\n",write_filename);

            do
            {
                err = unzReadCurrentFile(uf,buf,size_buf);
                if (err<0)
                {
                    printf("error %d with zipfile in unzReadCurrentFile\n",err);
                    break;
                }
                if (err>0)
                    if (fwrite(buf,(unsigned)err,1,fout)!=1)
                    {
                        printf("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        break;
                    }
            }
            while (err>0);
            if (fout)
                    fclose(fout);

            /*if (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);*/
        }

        if (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            if (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        else
            unzCloseCurrentFile(uf); /* don't lose the error */


    free(buf);
    return err;
}
