// ----------------------------------------------------------------------- //
//  This file is owned and controlled by Xilinx and must be used solely    //
//  for design, simulation, implementation and creation of design files    //
//  limited to Xilinx devices or technologies. Use with non-Xilinx         //
//  devices or technologies is expressly prohibited and immediately        //
//  terminates your license.                                               //
//                                                                         //
//  XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" SOLELY   //
//  FOR USE IN DEVELOPING PROGRAMS AND SOLUTIONS FOR XILINX DEVICES.  BY   //
//  PROVIDING THIS DESIGN, CODE, OR INFORMATION AS ONE POSSIBLE            //
//  IMPLEMENTATION OF THIS FEATURE, APPLICATION OR STANDARD, XILINX IS     //
//  MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION IS FREE FROM ANY     //
//  CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE FOR OBTAINING ANY      //
//  RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY      //
//  DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE  //
//  IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR         //
//  REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF        //
//  INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A  //
//  PARTICULAR PURPOSE.                                                    //
//                                                                         //
//  Xilinx products are not intended for use in life support appliances,   //
//  devices, or systems.  Use in such applications are expressly           //
//  prohibited.                                                            //
//                                                                         //
//  (c) Copyright 1995-2019 Xilinx, Inc.                                   //
//  All rights reserved.                                                   //
// ----------------------------------------------------------------------- //

#include <core.p4>
#include <xsa.p4>

/*
 * Forward Switch:
 * 
 * The forward design exemplifies the implementation the core of an IPv4/IPv6
 * network switch. IP destination address is used to perform an LPM search to 
 * determine the port where the packet needs to be redirected to. The IPv6 
 * table is setup to be implemented with an ternary CAM and the IPv4 table 
 * with a semi-ternary CAM.
 *
 */

typedef bit<48>  MacAddr;
typedef bit<32>  IPv4Addr;
typedef bit<128> IPv6Addr;

const bit<16> VLAN_TYPE  = 0x8100;
const bit<16> IPV4_TYPE  = 0x0800;
const bit<16> IPV6_TYPE  = 0x86DD;

const bit<8> TCP_PROT  = 0x06;
const bit<8> UDP_PROT  = 0x11;

// ****************************************************************************** //
// *************************** H E A D E R S  *********************************** //
// ****************************************************************************** //

header eth_mac_t {
    MacAddr dmac; // Destination MAC address
    MacAddr smac; // Source MAC address
    bit<16> type; // Tag Protocol Identifier
}

header vlan_t {
    bit<3>  pcp;  // Priority code point
    bit<1>  cfi;  // Drop eligible indicator
    bit<12> vid;  // VLAN identifier
    bit<16> tpid; // Tag protocol identifier
}

header ipv4_t {
    bit<4>   version;  // Version (4 for IPv4)
    bit<4>   hdr_len;  // Header length in 32b words
    bit<8>   tos;      // Type of Service
    bit<16>  length;   // Packet length in 32b words
    bit<16>  id;       // Identification
    bit<3>   flags;    // Flags
    bit<13>  offset;   // Fragment offset
    bit<8>   ttl;      // Time to live
    bit<8>   protocol; // Next protocol
    bit<16>  hdr_chk;  // Header checksum
    IPv4Addr src;      // Source address
    IPv4Addr dst;      // Destination address
}

header ipv4_opt_t {
    varbit<320> options; // IPv4 options - length = (ipv4.hdr_len - 5) * 32
}

header ipv6_t {
    bit<4>   version;    // Version = 6
    bit<8>   priority;   // Traffic class
    bit<20>  flow_label; // Flow label
    bit<16>  length;     // Payload length
    bit<8>   protocol;   // Next protocol
    bit<8>   hop_limit;  // Hop limit
    IPv6Addr src;        // Source address
    IPv6Addr dst;        // Destination address
}

header tcp_t {
    bit<16> src_port;   // Source port
    bit<16> dst_port;   // Destination port
    bit<32> seqNum;     // Sequence number
    bit<32> ackNum;     // Acknowledgment number
    bit<4>  dataOffset; // Data offset
    bit<6>  resv;       // Offset
    bit<6>  flags;      // Flags
    bit<16> window;     // Window
    bit<16> checksum;   // TCP checksum
    bit<16> urgPtr;     // Urgent pointer
}

header tcp_opt_t {
    varbit<320> options; // TCP options - length = (tcp.dataOffset - 5) * 32
}

header udp_t {
    bit<16> src_port;  // Source port
    bit<16> dst_port;  // Destination port
    bit<16> length;    // UDP length
    bit<16> checksum;  // UDP checksum
}

// ****************************************************************************** //
// ************************* S T R U C T U R E S  ******************************* //
// ****************************************************************************** //

// header structure
struct headers {
    eth_mac_t    eth;
    vlan_t[2]    vlan;
    ipv4_t       ipv4;
    ipv4_opt_t   ipv4opt;
    ipv6_t       ipv6;
    tcp_t        tcp;
    tcp_opt_t    tcpopt;
    udp_t        udp;
}

// User metadata structure
struct metadata {
    //bit<9> port;
	bit<16> tuser_size;
	bit<16> tuser_src;
	bit<16> tuser_dst;
}

// User-defined errors 
error {
    InvalidIPpacket,
    InvalidTCPpacket
}

// ****************************************************************************** //
// *************************** P A R S E R  ************************************* //
// ****************************************************************************** //

parser MyParser(packet_in packet, 
                out headers hdr, 
                inout metadata meta, 
                inout standard_metadata_t smeta) {
    
    state start {
        transition parse_eth;
    }
    
    state parse_eth {
        packet.extract(hdr.eth);
        transition select(hdr.eth.type) {
            VLAN_TYPE : parse_vlan;
            IPV4_TYPE : parse_ipv4;
            IPV6_TYPE : parse_ipv6;
            default   : accept; 
        }
    }
    
    state parse_vlan {
        packet.extract(hdr.vlan.next);
        transition select(hdr.vlan.last.tpid) {
            VLAN_TYPE : parse_vlan;
            IPV4_TYPE : parse_ipv4;
            IPV6_TYPE : parse_ipv6;
            default   : accept; 
        }
    }
    
    state parse_ipv4 {
        packet.extract(hdr.ipv4);
        verify(hdr.ipv4.version == 4 && hdr.ipv4.hdr_len >= 5, error.InvalidIPpacket);
        packet.extract(hdr.ipv4opt, (((bit<32>)hdr.ipv4.hdr_len - 5) * 32));
        transition select(hdr.ipv4.protocol) {
            TCP_PROT  : parse_tcp;
            UDP_PROT  : parse_udp;
            default   : accept; 
        }
    }
    
    state parse_ipv6 {
        packet.extract(hdr.ipv6);
        verify(hdr.ipv6.version == 6, error.InvalidIPpacket);
        transition select(hdr.ipv6.protocol) {
            TCP_PROT  : parse_tcp;
            UDP_PROT  : parse_udp;
            default   : accept; 
        } 
    }

    state parse_tcp {
        packet.extract(hdr.tcp);
        verify(hdr.tcp.dataOffset >= 5, error.InvalidTCPpacket);
        packet.extract(hdr.tcpopt, (((bit<32>)hdr.tcp.dataOffset - 5) * 32));
        transition accept;
    }
    
    state parse_udp {
        packet.extract(hdr.udp);
        transition accept;
    }
}

// ****************************************************************************** //
// **************************  P R O C E S S I N G   **************************** //
// ****************************************************************************** //

control MyProcessing(inout headers hdr, 
                     inout metadata meta, 
                     inout standard_metadata_t smeta) {
                      
   // action forwardPacket(bit<9> port) {
   //     meta.port = port;
   // }

    action forwardPacket() {
    }
    
    action dropPacket() {
		smeta.drop = 1;
    }

    table forwardIPv4 {
        key             = { hdr.ipv4.dst : lpm; }
        actions         = { forwardPacket; 
                            dropPacket; }
        size            = 1024;
		num_masks       = 64;
        default_action  = forwardPacket;
    }

    table forwardIPv6 {
        key             = { hdr.ipv6.dst : lpm; }
        actions         = { forwardPacket; 
                            dropPacket; }
        size            = 1024;
        default_action  = forwardPacket;
    }

    apply {
        
        if (smeta.parser_error != error.NoError) {
            dropPacket();
            return;
        }
        
        if (hdr.ipv4.isValid())
            forwardIPv4.apply();
        else if (hdr.ipv6.isValid())
            forwardIPv6.apply();
        else
			forwardPacket();
        
    }
} 

// ****************************************************************************** //
// ***************************  D E P A R S E R  ******************************** //
// ****************************************************************************** //

control MyDeparser(packet_out packet, 
                   in headers hdr,
                   inout metadata meta, 
                   inout standard_metadata_t smeta) {
    apply {
        packet.emit(hdr.eth);
        packet.emit(hdr.vlan);
        packet.emit(hdr.ipv4);
        packet.emit(hdr.ipv4opt);
        packet.emit(hdr.ipv6);
        packet.emit(hdr.tcp);
        packet.emit(hdr.tcpopt);
        packet.emit(hdr.udp);
    }
}

// ****************************************************************************** //
// *******************************  M A I N  ************************************ //
// ****************************************************************************** //

XilinxPipeline(
    MyParser(), 
    MyProcessing(), 
    MyDeparser()
) main;
