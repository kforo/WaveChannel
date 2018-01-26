/*
#
# Copyright 2018, sohu ipc
# author: ChenFawang
# main.cc
# 2018-01-26
# mail: fawangchen@sohu-inc.com
*/
#ifndef __QUEUE_H__
#define __QUEUE_H__

typedef void QueueFd;

QueueFd *QueueCreate(void);

void QueueDestroy(QueueFd *fd);

void QueueClear(QueueFd *fd);

int QueueAppend(QueueFd *fd, void *item);

void *QueueRetrieve(QueueFd *fd);

int QueueLength(QueueFd *fd);


#endif
