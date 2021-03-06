/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Green Network Research Group
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Sayef Azad Sakin <sayefsakin[at]gmail[dot]com>
 *          a
 */

#ifndef WRAN_NET_DEVICE_H
#define WRAN_NET_DEVICE_H

#define BS_FLAG 0
#define SS_FLAG 1
#define COMMON_CONTROL_CHANNEL_NUMBER GetTotalChannels()

// all values are in seconds
#define REQUEST_TO_BEACON_INTERVAL 0.01 // 10ms
#define BEACON_TO_REQUEST_INTERVAL 0.1 // 100 ms
#define REQUEST_TO_SENSE_RESULT_INTERVAL 0.01 // 10ms
#define GET_SENSING_RESULT_RETRY_INTERVAL 0.5 // 500 ms
#define MAX_SCANNING_TIME 0.5 // in seconds

#define PACKET_SEPARATOR '|'
#define PACKET_TYPE_START_SENSING 0
#define PACKET_TYPE_SEND_SENSING_RESULT 1

#define MESSAGE_BODY_SEPARATOR '#'
#define MESSAGE_TYPE_SEND_PING 0
#define MESSAGE_TYPE_SEND_SENSE_RESULT 1

#include "ns3/net-device.h"
#include "ns3/event-id.h"
#include "ns3/mac48-address.h"
#include "ns3/cid.h"
#include "wran-connection.h"
#include "ns3/cid-factory.h"
#include "wran-mac-messages.h"
#include "ns3/dl-mac-messages.h"
#include "ns3/ul-mac-messages.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/log.h"
#include "wran-phy.h"
#include "wran-mac-header.h"

namespace ns3 {

class Node;
class Packet;
class TraceContext;
class TraceResolver;
class Channel;
class WranChannel;
class PacketBurst;
class WranBurstProfileManager;
class WranConnectionManager;
class WranServiceFlowManager;
class WranBandwidthManager;
class WranUplinkScheduler;

/**
 * \defgroup wran WiMAX Models
 *
 * This section documents the API of the ns-3 wran module. For a generic functional description, please refer to the ns-3 manual.
 */

/**
 * \brief Hold together all Wran-related objects in a NetDevice.
 * \ingroup wran
 *
 * This class holds together ns3::WranPhy, ns3::WranConnection,
 * ns3::ConectionManager, ns3::WranBurstProfileManager, and 
 * ns3::WranBandwidthManager.
 */
class WranNetDevice : public NetDevice
{
public:
  enum Direction
  {
    DIRECTION_DOWNLINK, DIRECTION_UPLINK
  };

  enum RangingStatus
  {
    RANGING_STATUS_EXPIRED, RANGING_STATUS_CONTINUE, RANGING_STATUS_ABORT, RANGING_STATUS_SUCCESS
  };

  static TypeId GetTypeId (void);
  WranNetDevice (void);
  virtual ~WranNetDevice (void);
  /**
   * \param ttg transmit/receive transition gap
   */
  void SetTtg (uint16_t ttg);
  /**
   * \returns transmit/receive transition gap
   */
  uint16_t GetTtg (void) const;
  /**
   * \param rtg receive/transmit transition gap
   */
  void SetRtg (uint16_t rtg);
  /**
   * \returns receive/transmit transition gap
   */
  uint16_t GetRtg (void) const;
  void Attach (Ptr<WranChannel> channel);
  /**
   * \param phy the phy layer to use.
   */
  void SetPhy (Ptr<WranPhy> phy);
  /**
   * \returns a pointer to the physical layer.
   */
  Ptr<WranPhy> GetPhy (void) const;

  /**
   * \param wranChannel the channel to be used
   */
  void SetChannel (Ptr<WranChannel> wranChannel);

  uint64_t GetChannel (uint8_t index) const;

  void SetNrFrames (uint32_t nrFrames);
  uint32_t GetNrFrames (void) const;
  /**
   * \param address the mac address of the net device
   */
  void SetMacAddress (Mac48Address address);
  /**
   * \returns the mac address of the net device
   */
  Mac48Address GetMacAddress (void) const;
  void SetState (uint8_t state);
  uint8_t GetState (void) const;
  /**
   * \returns the initial ranging connection
   */
  Ptr<WranConnection> GetInitialRangingConnection (void) const;
  /**
    * \returns the broadcast connection
    */
  Ptr<WranConnection> GetBroadcastConnection (void) const;

  void SetCurrentDcd (Dcd dcd);
  Dcd GetCurrentDcd (void) const;
  void SetCurrentUcd (Ucd ucd);
  Ucd GetCurrentUcd (void) const;
  /**
   * \returns the connection manager of the device
   */
  Ptr<WranConnectionManager> GetWranConnectionManager (void) const;

  /**
   * \param  connectionManager the commection manager to be installed in the device
   */
  virtual void SetWranConnectionManager (Ptr<WranConnectionManager> connectionManager );

  /**
   * \returns the burst profile manager currently installed in the device
   */
  Ptr<WranBurstProfileManager> GetWranBurstProfileManager (void) const;

  /**
   * \param burstProfileManager the burst profile manager to be installed on the device
   */
  void SetWranBurstProfileManager (Ptr<WranBurstProfileManager> burstProfileManager);

  /**
   * \returns the bandwidth manager installed on the device
   */
  Ptr<WranBandwidthManager> GetWranBandwidthManager (void) const;

  /**
   * \param bandwidthManager the bandwidth manager to be installed on the device
   */
  void SetWranBandwidthManager (Ptr<WranBandwidthManager> bandwidthManager);

  /*
   * \brief Creates the initial ranging and broadcast connections
   */
  void CreateDefaultConnections (void);

  virtual void Start (void) = 0;
  virtual void Stop (void) = 0;

  void SetReceiveCallback (void);

  uint8_t GetTotalChannels (void);
  void SetTotalChannels (uint8_t t_channels);

  void ForwardUp (Ptr<Packet> packet, const Mac48Address &source, const Mac48Address &dest);

  virtual bool Enqueue (Ptr<Packet> packet, const MacHeaderType &hdrType, Ptr<WranConnection> connection) = 0;
  void ForwardDown (Ptr<PacketBurst> burst, WranPhy::ModulationType modulationType);

  // temp, shall be private
  static uint8_t m_direction; // downlink or uplink

  static Time m_frameStartTime; // temp, to determine the frame start time at SS side, shall actually be determined by frame start preamble

  virtual void SetName (const std::string name);
  virtual std::string GetName (void) const;
  virtual void SetIfIndex (const uint32_t index);
  virtual uint32_t GetIfIndex (void) const;
  virtual Ptr<Channel> GetPhyChannel (void) const;
  virtual Ptr<Channel> GetChannel (void) const;
  virtual void SetAddress (Address address);
  virtual Address GetAddress (void) const;
  virtual bool SetMtu (const uint16_t mtu);
  virtual uint16_t GetMtu (void) const;
  virtual bool IsLinkUp (void) const;
  virtual void SetLinkChangeCallback (Callback<void> callback);
  virtual bool IsBroadcast (void) const;
  virtual Address GetBroadcast (void) const;
  virtual bool IsMulticast (void) const;
  virtual Address GetMulticast (void) const;
  virtual Address MakeMulticastAddress (Ipv4Address multicastGroup) const;
  virtual bool IsPointToPoint (void) const;
  virtual bool Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);
  virtual void SetNode (Ptr<Node> node);
  virtual Ptr<Node> GetNode (void) const;
  virtual bool NeedsArp (void) const;
  virtual void SetReceiveCallback (NetDevice::ReceiveCallback cb);
  virtual void AddLinkChangeCallback (Callback<void> callback);
  virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);
  virtual void SetPromiscReceiveCallback (PromiscReceiveCallback cb);
  NetDevice::PromiscReceiveCallback GetPromiscReceiveCallback (void);
  virtual bool SupportsSendFrom (void) const;

  TracedCallback<Ptr<const Packet>, const Mac48Address&> m_traceRx;
  TracedCallback<Ptr<const Packet>, const Mac48Address&> m_traceTx;

  virtual void DoDispose (void);
  virtual Address GetMulticast (Ipv6Address addr) const;
  virtual Address GetMulticast (Ipv4Address multicastGroup) const;
  virtual bool IsBridge (void) const;

  bool IsPromisc (void);
  void NotifyPromiscTrace (Ptr<Packet> p);

private:
  WranNetDevice (const WranNetDevice &);
  WranNetDevice & operator= (const WranNetDevice &);

  static const uint16_t MAX_MSDU_SIZE = 1500;
  // recommended by wran forum.
  static const uint16_t DEFAULT_MSDU_SIZE = 1400;

  virtual bool DoSend (Ptr<Packet> packet,
                       const Mac48Address& source,
                       const Mac48Address& dest,
                       uint16_t protocolNumber) = 0;
  virtual void DoReceive (Ptr<Packet> packet) = 0;
  virtual Ptr<WranChannel> DoGetChannel (void) const;
  void Receive (Ptr<const PacketBurst> burst);
  void InitializeChannels (void);

  Ptr<Node> m_node;
  Ptr<WranPhy> m_phy;
  NetDevice::ReceiveCallback m_forwardUp;
  NetDevice::PromiscReceiveCallback m_promiscRx;

  uint32_t m_ifIndex;
  std::string m_name;
  bool m_linkUp;
  Callback<void> m_linkChange;
  mutable uint16_t m_mtu;

  // temp, shall be in BS. defined here to allow SS to access. SS shall actually determine it from DLFP, shall be moved to BS after DLFP is implemented
  static uint32_t m_nrFrames;

  // not sure if it shall be included here
  std::vector<uint64_t> m_dlChannels;

  Mac48Address m_address;
  uint8_t m_state;
  uint32_t m_symbolIndex;

  // length of TTG and RTG, in units of PSs
  uint16_t m_ttg;
  uint16_t m_rtg;

  Dcd m_currentDcd;
  Ucd m_currentUcd;

  Ptr<WranConnection> m_initialRangingConnection;
  Ptr<WranConnection> m_broadcastConnection;

  Ptr<WranConnectionManager> m_connectionManager;
  Ptr<WranBurstProfileManager> m_burstProfileManager;
  Ptr<WranBandwidthManager> m_bandwidthManager;

  Ptr<Object> m_mobility;

  uint8_t totalChannels;
};

} // namespace ns3

#endif /* WRAN_NET_DEVICE_H */
