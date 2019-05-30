/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2019 Philippe Gerum  <rpm@xenomai.org>
 */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <evl/thread.h>
#include <evl/syscall.h>
#include <evl/utils.h>
#include <uapi/evl/sched.h>
#include <uapi/evl/control.h>
#include "internal.h"

ssize_t evl_log(int proxyfd, const void *buf, size_t len)
{
	if (evl_is_inband())
		return write(proxyfd, buf, len);

	return oob_write(proxyfd, buf, len);
}

int evl_printf(int proxyfd, const char *fmt, ...)
{
	ssize_t len = EVL_PRINTBUF_SIZE;
	char *buf = evl_logging_buf;
	va_list ap;

	va_start(ap, fmt);
	len = vsnprintf(buf, len, fmt, ap);
	va_end(ap);

	return evl_log(proxyfd, buf, len);
}

int evl_sched_control(int policy,
		union evl_sched_ctlparam *param,
		union evl_sched_ctlinfo *info,
		int cpu)
{
	struct evl_sched_ctlreq ctlreq;
	int ret;

	if (evl_ctlfd < 0)
		return -ENXIO;

	ctlreq.policy = policy;
	ctlreq.cpu = cpu;
	ctlreq.param = param;
	ctlreq.info = info;

	if (evl_is_inband())
		ret = ioctl(evl_ctlfd, EVL_CTLIOC_SCHEDCTL, &ctlreq);
	else
		ret = oob_ioctl(evl_ctlfd, EVL_CTLIOC_SCHEDCTL, &ctlreq);

	return ret ? -errno : 0;
}
