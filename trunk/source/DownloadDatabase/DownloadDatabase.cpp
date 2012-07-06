#include "StdAfx.h"
#include "DownloadDatabase.h"

CDownloadDatabase::CDownloadDatabase(const char* proxy, int begin, int end, int con_num)
: m_iBegin(begin)
, m_iEnd(end)
, m_iConNum(con_num)
, m_iDispatchedNum(0)
, m_iDownloadedNum(0)
, m_ThreadHandles(NULL)
, m_ThreadInfo(NULL)
, m_pFile(NULL)
{
    if(proxy)
    {
        strcpy(m_Proxy, proxy);
        m_bProxy = true;
    }
    else
    {
        m_bProxy = false;
    }
    m_iBegin    = begin;
    m_iEnd      = end;
    m_iConNum   = con_num;
}

int CDownloadDatabase::Run()
{
    int active_num, active_order, ret;
    int group, order;

    printf("Download Percent:0.00%%\r");

    while(1)
    {
        DispatchTask();
        active_num = SortThread();
        if(active_num <= 0) break;
        ret = WaitForMultipleObjects(active_num, m_ThreadHandles, false, INFINITE);
        active_order = ret - WAIT_OBJECT_0;
        if(active_order >= active_num) continue;
        SAFE_CLOSEHANDLE(m_ThreadHandles[active_order]);
        group = m_ThreadInfo[active_order].group;
        order = m_ThreadInfo[active_order].order;
        if(m_Data[group][order].state != EBUFSTATE_DOWNLOADED) return 0;
        m_Data[group][order].state = EBUFSTATE_FINISH;
        m_iDownloadedNum++;
        printf("Download Percent:%.2f%%\r", m_iDownloadedNum * 100.0f / m_iPuzNum);
        if(group != m_iFrontGroup) continue;
        if(!IsAllFinish(m_iFrontGroup)) continue;
        for(int i = 0; i < m_iDataLen; i++)
        {
            if(m_Data[m_iFrontGroup][i].state == EBUFSTATE_FINISH)
            {
                fputs(m_Data[m_iFrontGroup][i].buf, m_pFile);
                fputs("\n", m_pFile);
            }
            m_Data[m_iFrontGroup][i].state = EBUFSTATE_UNDISPATCH;
        }
        m_iDataUsed[m_iFrontGroup]  = 0;
        m_iFrontGroup               = 1 - m_iFrontGroup;
        m_iBackGroup                = 1 - m_iBackGroup;
    }
    return 1;
}

int CDownloadDatabase::Init()
{
    m_iPuzNum           = m_iEnd - m_iBegin + 1;
    m_iConNum           = min(m_iConNum, m_iPuzNum);
    m_iDataLen          = m_iConNum * DATAMULT;
    m_ThreadHandles     = new HANDLE[m_iConNum];
    m_ThreadInfo        = new ThreadInfo[m_iConNum];
    m_Data[0]           = new Data[m_iDataLen];
    m_Data[1]           = new Data[m_iDataLen];
    m_iDataUsed[0]      = 0;
    m_iDataUsed[1]      = 0;
    m_iFrontGroup       = 0;
    m_iBackGroup        = 1;

    for(int i = 0; i < m_iConNum; i++)
    {
        m_ThreadHandles[i] = NULL;
        if(m_bProxy) m_ThreadInfo[i].proxy = m_Proxy;
        else m_ThreadInfo[i].proxy = NULL;
    }
    for(int i = 0; i < m_iDataLen; i++)
    {
        m_Data[0][i].state = EBUFSTATE_UNDISPATCH;
        m_Data[1][i].state = EBUFSTATE_UNDISPATCH;
    }

    char filename[32];
    sprintf(filename, "%d-%d.txt", m_iBegin, m_iEnd);
    DeleteFile(filename);
    m_pFile = fopen(filename, "w");
    if(!m_pFile) ERROR_RTN0("Can\'t open out file!");

    return 1;
}

void CDownloadDatabase::Release()
{
    SAFE_DELETEA(m_ThreadHandles);
    SAFE_DELETEA(m_ThreadInfo);
    SAFE_DELETEA(m_Data[0]);
    SAFE_DELETEA(m_Data[1]);
    SAFE_FCLOSE(m_pFile);
}

void CDownloadDatabase::DispatchTask()
{
    for(int i = 0; i < m_iConNum; i++)
    {
        if(m_iDispatchedNum >= m_iPuzNum) break;
        if(m_ThreadHandles[i]) continue;

        if(m_iDataUsed[m_iFrontGroup] < m_iDataLen)
        {
            m_ThreadInfo[i].group = m_iFrontGroup;
            m_ThreadInfo[i].order = m_iDataUsed[m_iFrontGroup]++;
        }
        else if(m_iDataUsed[m_iBackGroup] < m_iDataLen)
        {
            m_ThreadInfo[i].group = m_iBackGroup;
            m_ThreadInfo[i].order = m_iDataUsed[m_iBackGroup]++;
        }
        else
        {
            break;
        }

        Data* pData = &(m_Data[m_ThreadInfo[i].group][m_ThreadInfo[i].order]);
        pData->state = EBUFSTATE_DISPATCHED;
        m_ThreadInfo[i].data = pData;
        m_ThreadInfo[i].puzzle = m_iBegin + m_iDispatchedNum;
        m_iDispatchedNum++;
        m_ThreadHandles[i] = CreateThread(NULL, 0, Download, &(m_ThreadInfo[i]), 0, NULL);
    }
}

int CDownloadDatabase::SortThread()
{
    int active_num = 0;
    int lastvalid = m_iConNum;
    for(int i = 0; i < lastvalid; i++)
    {
        if(m_ThreadHandles[i])
        {
            active_num++;
            continue;
        }

        for(int j = lastvalid - 1; j >= 0; j--)
        {
            if(m_ThreadHandles[j] == NULL) continue;
            lastvalid = j;
            break;
        }

        if(lastvalid <= i) break;
        SwapThread(i, lastvalid);
        active_num++;
    }

    return active_num;
}

void CDownloadDatabase::SwapThread(int i, int j)
{
    if(i < 0 || i > m_iConNum || j < 0 || j > m_iConNum) return;

    HANDLE handle = m_ThreadHandles[i];
    ThreadInfo info = m_ThreadInfo[i];

    m_ThreadHandles[i] = m_ThreadHandles[j];
    m_ThreadInfo[i]    = m_ThreadInfo[j];
    m_ThreadHandles[j] = handle;
    m_ThreadInfo[j]    = info;
}

bool CDownloadDatabase::IsAllFinish(int group)
{
    if(group < 0 || group  >= 2) return false;
    for(int i = 0; i < m_iDataLen; i++)
    {
        if(m_Data[group][i].state == EBUFSTATE_FINISH) continue;
        if(m_iDispatchedNum >= m_iPuzNum && m_Data[group][i].state == EBUFSTATE_UNDISPATCH) continue;

        return false;
    }

    return true;
}

DWORD WINAPI CDownloadDatabase::Download(void* param)
{
    ThreadInfo* info = (ThreadInfo*)param;

    char temp_name[32];
    sprintf(temp_name, "temp%d.txt", info->puzzle);

    char commond_buf[256];
    if(info->proxy)
    {
        sprintf(commond_buf, "tool\\wget -e \"http_proxy = %s\" -q -O %s http://www.suduko.us/j/smallcn.php?xh=%d", info->proxy, temp_name, info->puzzle);
    }
    else
    {
        sprintf(commond_buf, "tool\\wget -q -O %s http://www.suduko.us/j/smallcn.php?xh=%d", temp_name, info->puzzle);
    }

    //WinExec(commond_buf, SW_HIDE);
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb   = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    CreateProcess(NULL, commond_buf, NULL, NULL, true, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);

    WaitForSingleObject(pi.hProcess, INFINITE);

    FILE*   pfr   = fopen(temp_name, "r");
    if(!pfr) ERROR_RTN0("Can\'t download puzzle!");

    char    str_buf[BUFLEN];
    char    *p1, *p2;
    bool    bFound  = false;
    while(fgets(str_buf, BUFLEN, pfr))
    {
        p1  = StrStr(str_buf, "tmda=\'");
        if(!p1) continue;
        p1  += 6;
        p2  = StrStr(p1, "\'");
        if(!p2) break;
        *p2 = 0;
        bFound  = true;
        break;
    }
    fclose(pfr);
    DeleteFile(temp_name);
    if(!bFound) ERROR_RTN0("Can\'t find puzzle!");

    strcpy(info->data->buf, p1);
    info->data->state = EBUFSTATE_DOWNLOADED;

    Sleep(1000);

    return 1;
}