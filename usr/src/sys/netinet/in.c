/*
 * Copyright (c) 1982, 1986, 1991 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)in.c	7.26 (Berkeley) %G%
 */

#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/malloc.h>
#include <sys/socket.h>
#include <sys/socketvar.h>

#include <net/if.h>
#include <net/route.h>
#include <net/af.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/in_var.h>

#ifdef INET
/*
 * Formulate an Internet address from network + host.
 */
struct in_addr
in_makeaddr(net, host)
	u_long net, host;
{
	register struct in_ifaddr *ia;
	register u_long mask;
	u_long addr;

	if (IN_CLASSA(net))
		mask = IN_CLASSA_HOST;
	else if (IN_CLASSB(net))
		mask = IN_CLASSB_HOST;
	else
		mask = IN_CLASSC_HOST;
	for (ia = in_ifaddr; ia; ia = ia->ia_next)
		if ((ia->ia_netmask & net) == ia->ia_net) {
			mask = ~ia->ia_subnetmask;
			break;
		}
	addr = htonl(net | (host & mask));
	return (*(struct in_addr *)&addr);
}

/*
 * Return the network number from an internet address.
 */
u_long
in_netof(in)
	struct in_addr in;
{
	register u_long i = ntohl(in.s_addr);
	register u_long net;
	register struct in_ifaddr *ia;

	if (IN_CLASSA(i))
		net = i & IN_CLASSA_NET;
	else if (IN_CLASSB(i))
		net = i & IN_CLASSB_NET;
	else if (IN_CLASSC(i))
		net = i & IN_CLASSC_NET;
#ifdef MULTICAST
	else if (IN_CLASSD(i))
		net = i & IN_CLASSD_NET;
#endif
	else
		return (0);

	/*
	 * Check whether network is a subnet;
	 * if so, return subnet number.
	 */
	for (ia = in_ifaddr; ia; ia = ia->ia_next)
		if (net == ia->ia_net)
			return (i & ia->ia_subnetmask);
	return (net);
}

/*
 * Compute and save network mask as sockaddr from an internet address.
 */
in_sockmaskof(in, sockmask)
	struct in_addr in;
	register struct sockaddr_in *sockmask;
{
	register u_long net;
	register u_long mask;
    {
	register u_long i = ntohl(in.s_addr);

	if (i == 0)
		net = 0, mask = 0;
	else if (IN_CLASSA(i))
		net = i & IN_CLASSA_NET, mask = IN_CLASSA_NET;
	else if (IN_CLASSB(i))
		net = i & IN_CLASSB_NET, mask = IN_CLASSB_NET;
	else if (IN_CLASSC(i))
		net = i & IN_CLASSC_NET, mask = IN_CLASSC_NET;
	else
		net = i, mask = -1;
    }
    {
	register struct in_ifaddr *ia;
	/*
	 * Check whether network is a subnet;
	 * if so, return subnet number.
	 */
	for (ia = in_ifaddr; ia; ia = ia->ia_next)
		if (net == ia->ia_net)
			mask =  ia->ia_subnetmask;
    }
    {
	register char *cpbase = (char *)&(sockmask->sin_addr);
	register char *cp = (char *)(1 + &(sockmask->sin_addr));

	sockmask->sin_addr.s_addr = htonl(mask);
	sockmask->sin_len = 0;
	while (--cp >= cpbase)
		if (*cp) {
			sockmask->sin_len = 1 + cp - (caddr_t)sockmask;
			break;
		}
    }
}

/*
 * Return the host portion of an internet address.
 */
u_long
in_lnaof(in)
	struct in_addr in;
{
	register u_long i = ntohl(in.s_addr);
	register u_long net, host;
	register struct in_ifaddr *ia;

	if (IN_CLASSA(i)) {
		net = i & IN_CLASSA_NET;
		host = i & IN_CLASSA_HOST;
	} else if (IN_CLASSB(i)) {
		net = i & IN_CLASSB_NET;
		host = i & IN_CLASSB_HOST;
	} else if (IN_CLASSC(i)) {
		net = i & IN_CLASSC_NET;
		host = i & IN_CLASSC_HOST;
#ifdef MULTICAST
	} else if (IN_CLASSD(i)) {
		net = i & IN_CLASSD_NET;
		host = i & IN_CLASSD_HOST;
#endif
	} else
		return (i);

	/*
	 * Check whether network is a subnet;
	 * if so, use the modified interpretation of `host'.
	 */
	for (ia = in_ifaddr; ia; ia = ia->ia_next)
		if (net == ia->ia_net)
			return (host &~ ia->ia_subnetmask);
	return (host);
}

#ifndef SUBNETSARELOCAL
#define	SUBNETSARELOCAL	1
#endif
int subnetsarelocal = SUBNETSARELOCAL;
/*
 * Return 1 if an internet address is for a ``local'' host
 * (one to which we have a connection).  If subnetsarelocal
 * is true, this includes other subnets of the local net.
 * Otherwise, it includes only the directly-connected (sub)nets.
 */
in_localaddr(in)
	struct in_addr in;
{
	register u_long i = ntohl(in.s_addr);
	register struct in_ifaddr *ia;

	if (subnetsarelocal) {
		for (ia = in_ifaddr; ia; ia = ia->ia_next)
			if ((i & ia->ia_netmask) == ia->ia_net)
				return (1);
	} else {
		for (ia = in_ifaddr; ia; ia = ia->ia_next)
			if ((i & ia->ia_subnetmask) == ia->ia_subnet)
				return (1);
	}
	return (0);
}

/*
 * Determine whether an IP address is in a reserved set of addresses
 * that may not be forwarded, or whether datagrams to that destination
 * may be forwarded.
 */
in_canforward(in)
	struct in_addr in;
{
	register u_long i = ntohl(in.s_addr);
	register u_long net;

	if (IN_EXPERIMENTAL(i))
		return (0);
	if (IN_CLASSA(i)) {
		net = i & IN_CLASSA_NET;
		if (net == 0 || net == IN_LOOPBACKNET)
			return (0);
	}
	return (1);
}

int	in_interfaces;		/* number of external internet interfaces */
extern	struct ifnet loif;

/*
 * Generic internet control operations (ioctl's).
 * Ifp is 0 if not an interface-specific ioctl.
 */
/* ARGSUSED */
in_control(so, cmd, data, ifp)
	struct socket *so;
	int cmd;
	caddr_t data;
	register struct ifnet *ifp;
{
	register struct ifreq *ifr = (struct ifreq *)data;
	register struct in_ifaddr *ia = 0;
	register struct ifaddr *ifa;
	struct in_ifaddr *oia;
	struct in_aliasreq *ifra = (struct in_aliasreq *)data;
	struct sockaddr_in oldaddr;
	int error, hostIsNew, maskIsNew;
	u_long i;

	/*
	 * Find address for this interface, if it exists.
	 */
	if (ifp)
		for (ia = in_ifaddr; ia; ia = ia->ia_next)
			if (ia->ia_ifp == ifp)
				break;

	switch (cmd) {

	case SIOCAIFADDR:
	case SIOCDIFADDR:
		if (ifra->ifra_addr.sin_family == AF_INET)
		    for (oia = ia; ia; ia = ia->ia_next) {
			if (ia->ia_ifp == ifp  &&
			    ia->ia_addr.sin_addr.s_addr ==
				ifra->ifra_addr.sin_addr.s_addr)
			    break;
		}
		if (cmd == SIOCDIFADDR && ia == 0)
			return (EADDRNOTAVAIL);
		/* FALLTHROUGH */
	case SIOCSIFADDR:
	case SIOCSIFNETMASK:
	case SIOCSIFDSTADDR:
		if ((so->so_state & SS_PRIV) == 0)
			return (EPERM);

		if (ifp == 0)
			panic("in_control");
		if (ia == (struct in_ifaddr *)0) {
			oia = (struct in_ifaddr *)
				malloc(sizeof *oia, M_IFADDR, M_WAITOK);
			if (oia == (struct in_ifaddr *)NULL)
				return (ENOBUFS);
			bzero((caddr_t)oia, sizeof *oia);
			if (ia = in_ifaddr) {
				for ( ; ia->ia_next; ia = ia->ia_next)
					continue;
				ia->ia_next = oia;
			} else
				in_ifaddr = oia;
			ia = oia;
			if (ifa = ifp->if_addrlist) {
				for ( ; ifa->ifa_next; ifa = ifa->ifa_next)
					continue;
				ifa->ifa_next = (struct ifaddr *) ia;
			} else
				ifp->if_addrlist = (struct ifaddr *) ia;
			ia->ia_ifa.ifa_addr = (struct sockaddr *)&ia->ia_addr;
			ia->ia_ifa.ifa_dstaddr
					= (struct sockaddr *)&ia->ia_dstaddr;
			ia->ia_ifa.ifa_netmask
					= (struct sockaddr *)&ia->ia_sockmask;
			ia->ia_sockmask.sin_len = 8;
			if (ifp->if_flags & IFF_BROADCAST) {
				ia->ia_broadaddr.sin_len = sizeof(ia->ia_addr);
				ia->ia_broadaddr.sin_family = AF_INET;
			}
			ia->ia_ifp = ifp;
			if (ifp != &loif)
				in_interfaces++;
		}
		break;

	case SIOCSIFBRDADDR:
		if ((so->so_state & SS_PRIV) == 0)
			return (EPERM);
		/* FALLTHROUGH */

	case SIOCGIFADDR:
	case SIOCGIFNETMASK:
	case SIOCGIFDSTADDR:
	case SIOCGIFBRDADDR:
		if (ia == (struct in_ifaddr *)0)
			return (EADDRNOTAVAIL);
		break;
	}
	switch (cmd) {

	case SIOCGIFADDR:
		*((struct sockaddr_in *)&ifr->ifr_addr) = ia->ia_addr;
		break;

	case SIOCGIFBRDADDR:
		if ((ifp->if_flags & IFF_BROADCAST) == 0)
			return (EINVAL);
		*((struct sockaddr_in *)&ifr->ifr_dstaddr) = ia->ia_broadaddr;
		break;

	case SIOCGIFDSTADDR:
		if ((ifp->if_flags & IFF_POINTOPOINT) == 0)
			return (EINVAL);
		*((struct sockaddr_in *)&ifr->ifr_dstaddr) = ia->ia_dstaddr;
		break;

	case SIOCGIFNETMASK:
		*((struct sockaddr_in *)&ifr->ifr_addr) = ia->ia_sockmask;
		break;

	case SIOCSIFDSTADDR:
		if ((ifp->if_flags & IFF_POINTOPOINT) == 0)
			return (EINVAL);
		oldaddr = ia->ia_dstaddr;
		ia->ia_dstaddr = *(struct sockaddr_in *)&ifr->ifr_dstaddr;
		if (ifp->if_ioctl && (error = (*ifp->if_ioctl)
					(ifp, SIOCSIFDSTADDR, (caddr_t)ia))) {
			ia->ia_dstaddr = oldaddr;
			return (error);
		}
		if (ia->ia_flags & IFA_ROUTE) {
			ia->ia_ifa.ifa_dstaddr = (struct sockaddr *)&oldaddr;
			rtinit(&(ia->ia_ifa), (int)RTM_DELETE, RTF_HOST);
			ia->ia_ifa.ifa_dstaddr =
					(struct sockaddr *)&ia->ia_dstaddr;
			rtinit(&(ia->ia_ifa), (int)RTM_ADD, RTF_HOST|RTF_UP);
		}
		break;

	case SIOCSIFBRDADDR:
		if ((ifp->if_flags & IFF_BROADCAST) == 0)
			return (EINVAL);
		ia->ia_broadaddr = *(struct sockaddr_in *)&ifr->ifr_broadaddr;
		break;

	case SIOCSIFADDR:
		return (in_ifinit(ifp, ia,
		    (struct sockaddr_in *) &ifr->ifr_addr, 1));

	case SIOCSIFNETMASK:
		i = ifra->ifra_addr.sin_addr.s_addr;
		ia->ia_subnetmask = ntohl(ia->ia_sockmask.sin_addr.s_addr = i);
		break;

	case SIOCAIFADDR:
		maskIsNew = 0;
		hostIsNew = 1;
		error = 0;
		if (ia->ia_addr.sin_family == AF_INET) {
			if (ifra->ifra_addr.sin_len == 0) {
				ifra->ifra_addr = ia->ia_addr;
				hostIsNew = 0;
			} else if (ifra->ifra_addr.sin_addr.s_addr ==
					       ia->ia_addr.sin_addr.s_addr)
				hostIsNew = 0;
		}
		if (ifra->ifra_mask.sin_len) {
			in_ifscrub(ifp, ia);
			ia->ia_sockmask = ifra->ifra_mask;
			ia->ia_subnetmask =
			     ntohl(ia->ia_sockmask.sin_addr.s_addr);
			maskIsNew = 1;
		}
		if ((ifp->if_flags & IFF_POINTOPOINT) &&
		    (ifra->ifra_dstaddr.sin_family == AF_INET)) {
			in_ifscrub(ifp, ia);
			ia->ia_dstaddr = ifra->ifra_dstaddr;
			maskIsNew  = 1; /* We lie; but the effect's the same */
		}
		if (ifra->ifra_addr.sin_family == AF_INET &&
		    (hostIsNew || maskIsNew))
			error = in_ifinit(ifp, ia, &ifra->ifra_addr, 0);
		if ((ifp->if_flags & IFF_BROADCAST) &&
		    (ifra->ifra_broadaddr.sin_family == AF_INET))
			ia->ia_broadaddr = ifra->ifra_broadaddr;
		return (error);

	case SIOCDIFADDR:
		in_ifscrub(ifp, ia);
		if ((ifa = ifp->if_addrlist) == (struct ifaddr *)ia)
			ifp->if_addrlist = ifa->ifa_next;
		else {
			while (ifa->ifa_next &&
			       (ifa->ifa_next != (struct ifaddr *)ia))
				    ifa = ifa->ifa_next;
			if (ifa->ifa_next)
				ifa->ifa_next = ((struct ifaddr *)ia)->ifa_next;
			else
				printf("Couldn't unlink inifaddr from ifp\n");
		}
		oia = ia;
		if (oia == (ia = in_ifaddr))
			in_ifaddr = ia->ia_next;
		else {
			while (ia->ia_next && (ia->ia_next != oia))
				ia = ia->ia_next;
			if (ia->ia_next)
				ia->ia_next = oia->ia_next;
			else
				printf("Didn't unlink inifadr from list\n");
		}
		IFAFREE((&oia->ia_ifa));
		break;

	default:
		if (ifp == 0 || ifp->if_ioctl == 0)
			return (EOPNOTSUPP);
		return ((*ifp->if_ioctl)(ifp, cmd, data));
	}
	return (0);
}

/*
 * Delete any existing route for an interface.
 */
in_ifscrub(ifp, ia)
	register struct ifnet *ifp;
	register struct in_ifaddr *ia;
{

	if ((ia->ia_flags & IFA_ROUTE) == 0)
		return;
	if (ifp->if_flags & (IFF_LOOPBACK|IFF_POINTOPOINT))
		rtinit(&(ia->ia_ifa), (int)RTM_DELETE, RTF_HOST);
	else
		rtinit(&(ia->ia_ifa), (int)RTM_DELETE, 0);
	ia->ia_flags &= ~IFA_ROUTE;
}

/*
 * Initialize an interface's internet address
 * and routing table entry.
 */
in_ifinit(ifp, ia, sin, scrub)
	register struct ifnet *ifp;
	register struct in_ifaddr *ia;
	struct sockaddr_in *sin;
	int scrub;
{
	register u_long i = ntohl(sin->sin_addr.s_addr);
	struct sockaddr_in oldaddr;
	int s = splimp(), flags = RTF_UP, error, ether_output();
	void arp_rtrequest();

	oldaddr = ia->ia_addr;
	ia->ia_addr = *sin;
	/*
	 * Give the interface a chance to initialize
	 * if this is its first address,
	 * and to validate the address if necessary.
	 */
	if (ifp->if_ioctl &&
	    (error = (*ifp->if_ioctl)(ifp, SIOCSIFADDR, (caddr_t)ia))) {
		splx(s);
		ia->ia_addr = oldaddr;
		return (error);
	}
	if (ifp->if_output == ether_output) { /* XXX: Another Kludge */
		ia->ia_ifa.ifa_rtrequest = arp_rtrequest;
		ia->ia_ifa.ifa_flags |= RTF_CLONING;
	}
	splx(s);
	if (scrub) {
		ia->ia_ifa.ifa_addr = (struct sockaddr *)&oldaddr;
		in_ifscrub(ifp, ia);
		ia->ia_ifa.ifa_addr = (struct sockaddr *)&ia->ia_addr;
	}
	if (IN_CLASSA(i))
		ia->ia_netmask = IN_CLASSA_NET;
	else if (IN_CLASSB(i))
		ia->ia_netmask = IN_CLASSB_NET;
	else
		ia->ia_netmask = IN_CLASSC_NET;
	ia->ia_net = i & ia->ia_netmask;
	/*
	 * The subnet mask includes at least the standard network part,
	 * but may already have been set to a larger value.
	 */
	ia->ia_subnetmask |= ia->ia_netmask;
	ia->ia_subnet = i & ia->ia_subnetmask;
	ia->ia_sockmask.sin_addr.s_addr = htonl(ia->ia_subnetmask);
	{
		register char *cp = (char *) (1 + &(ia->ia_sockmask.sin_addr));
		register char *cpbase = (char *) &(ia->ia_sockmask.sin_addr);
		while (--cp >= cpbase)
			if (*cp) {
				ia->ia_sockmask.sin_len =
					1 + cp - (char *) &(ia->ia_sockmask);
				break;
			}
	}
	/*
	 * Add route for the network.
	 */
	ia->ia_ifa.ifa_metric = ifp->if_metric;
	if (ifp->if_flags & IFF_BROADCAST) {
		ia->ia_broadaddr.sin_addr = 
			in_makeaddr(ia->ia_subnet, INADDR_BROADCAST);
		ia->ia_netbroadcast.s_addr =
		    htonl(ia->ia_net | (INADDR_BROADCAST &~ ia->ia_netmask));
	} else if (ifp->if_flags & IFF_LOOPBACK) {
		ia->ia_ifa.ifa_dstaddr = ia->ia_ifa.ifa_addr;
		flags |= RTF_HOST;
	} else if (ifp->if_flags & IFF_POINTOPOINT) {
		if (ia->ia_dstaddr.sin_family != AF_INET)
			return (0);
		flags |= RTF_HOST;
	}
	if ((error = rtinit(&(ia->ia_ifa), (int)RTM_ADD, flags)) == 0)
		ia->ia_flags |= IFA_ROUTE;
#ifdef MULTICAST
	/*
	 * If the interface supports multicast, join the "all hosts"
	 * multicast group on that interface.
	 */
	if (ifp->if_flags & IFF_MULTICAST) {
		struct in_addr addr;

		addr.s_addr = htonl(INADDR_ALLHOSTS_GROUP);
		in_addmulti(&addr, ifp);
	}
#endif
	return (error);
}

/*
 * Return address info for specified internet network.
 */
struct in_ifaddr *
in_iaonnetof(net)
	u_long net;
{
	register struct in_ifaddr *ia;

	for (ia = in_ifaddr; ia; ia = ia->ia_next)
		if (ia->ia_subnet == net)
			return (ia);
	return ((struct in_ifaddr *)0);
}

/*
 * Return 1 if the address might be a local broadcast address.
 */
in_broadcast(in)
	struct in_addr in;
{
	register struct in_ifaddr *ia;
	u_long t;

	/*
	 * Look through the list of addresses for a match
	 * with a broadcast address.
	 */
	for (ia = in_ifaddr; ia; ia = ia->ia_next)
	    if (ia->ia_ifp->if_flags & IFF_BROADCAST) {
		if (ia->ia_broadaddr.sin_addr.s_addr == in.s_addr)
		     return (1);
		/*
		 * Check for old-style (host 0) broadcast.
		 */
		if ((t = ntohl(in.s_addr)) == ia->ia_subnet || t == ia->ia_net)
		    return (1);
	}
	if (in.s_addr == INADDR_BROADCAST || in.s_addr == INADDR_ANY)
		return (1);
	return (0);
}

#ifdef MULTICAST
/*
 * Add an address to the list of IP multicast addresses for a given interface.
 */
struct in_multi *
in_addmulti(ap, ifp)
	register struct in_addr *ap;
	register struct ifnet *ifp;
{
	register struct in_multi *inm;
	struct ifreq ifr;
	struct in_ifaddr *ia;
	int s = splnet();
int error;

	/*
	 * See if address already in list.
	 */
	IN_LOOKUP_MULTI(*ap, ifp, inm);
	if (inm != NULL) {
		/*
		 * Found it; just increment the reference count.
		 */
		++inm->inm_refcount;
	}
	else {
		/*
		 * New address; allocate a new multicast record
		 * and link it into the interface's multicast list.
		 */
		inm = (struct in_multi *)malloc(sizeof(*inm),
		    M_IPMADDR, M_NOWAIT);
		if (inm == NULL) {
			splx(s);
			return (NULL);
		}
		inm->inm_addr = *ap;
		inm->inm_ifp = ifp;
		inm->inm_refcount = 1;
		IFP_TO_IA(ifp, ia);
		if (ia == NULL) {
			free(inm, M_IPMADDR);
			splx(s);
			return (NULL);
		}
		inm->inm_ia = ia;
		inm->inm_next = ia->ia_multiaddrs;
		ia->ia_multiaddrs = inm;
		/*
		 * Ask the network driver to update its multicast reception
		 * filter appropriately for the new address.
		 */
		((struct sockaddr_in *)&ifr.ifr_addr)->sin_family = AF_INET;
		((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr = *ap;
		if (ifp->if_ioctl == NULL) {
			free(inm, M_IPMADDR);
			splx(s);
			return (NULL);
		}
		error = (*ifp->if_ioctl)(ifp, SIOCADDMULTI,(caddr_t)&ifr);
		if (error != 0) {
			free(inm, M_IPMADDR);
			splx(s);
			return (NULL);
		}
		/*
		 * Let IGMP know that we have joined a new IP multicast group.
		 */
		igmp_joingroup(inm);
	}
	splx(s);
	return (inm);
}

/*
 * Delete a multicast address record.
 */
int
in_delmulti(inm)
	register struct in_multi *inm;
{
	register struct in_multi **p;
	struct ifreq ifr;
	int s = splnet();

	if (--inm->inm_refcount == 0) {
		/*
		 * No remaining claims to this record; let IGMP know that
		 * we are leaving the multicast group.
		 */
		igmp_leavegroup(inm);
		/*
		 * Unlink from list.
		 */
		for (p = &inm->inm_ia->ia_multiaddrs;
		     *p != inm;
		     p = &(*p)->inm_next)
			 continue;
		*p = (*p)->inm_next;
		/*
		 * Notify the network driver to update its multicast reception
		 * filter.
		 */
		((struct sockaddr_in *)&(ifr.ifr_addr))->sin_family = AF_INET;
		((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr =
								inm->inm_addr;
		(*inm->inm_ifp->if_ioctl)(inm->inm_ifp, SIOCDELMULTI,
							     (caddr_t)&ifr);
		free(inm, M_IPMADDR);
	}
	splx(s);
}
#endif
#endif
