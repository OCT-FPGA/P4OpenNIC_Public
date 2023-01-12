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
 * P4 Calculator:
 *
 * This program implements a simple calculator, based on a custom protocol carried over Ethernet. 
 * The device uses the operation ID to perform an exact-match (with direct matching) and execute 
 * the requested operation. Packets with an unknown operation ID are dropped. 
 * The packet containing the result of the operation is sent back out of the same port it came in on,
 * while swapping the Ethernet source and destination addresses. The supported operations are: 
 * arithmetic addition, subtraction and multiplication, as well as bit-wise AND, OR and XOR.
 *
 * Custom Protocol format:
 *        0                 1                 2              3
 * +----------------+----------------+----------------+---------------+
 * |      P         |       4        |     Version    |     Op        |
 * +----------------+----------------+----------------+---------------+
 * |                              Operand A                           |
 * +----------------+----------------+----------------+---------------+
 * |                              Operand B                           |
 * +----------------+----------------+----------------+---------------+
 * |                              Result                              |
 * +----------------+----------------+----------------+---------------+
 *
 */

const bit<16> P4CALC_ETYPE = 0x1234; // custom value
const bit<8>  P4CALC_P     = 0x50;   // 'P'
const bit<8>  P4CALC_4     = 0x34;   // '4'
const bit<8>  P4CALC_VER   = 0x01;   // v0.1

// ****************************************************************************** //
// *************************** H E A D E R S  *********************************** //
// ****************************************************************************** //

header eth_mac_t {
    bit<48> dstAddr;     // Destination MAC address
    bit<48> srcAddr;     // Source MAC address
    bit<16> etherType;   // Tag Protocol Identifier
}

header p4calc_t {
    bit<8>  p;           // P is an ASCII Letter 'P'
    bit<8>  four;        // 4 is an ASCII Letter '4'
    bit<8>  version;     // Version is currently 0.1
    bit<5>  reserved;    // Padding
    bit<3>  operation;   // Op is an operation to Perform
    bit<32> operand_a;   // A operand (left)
    bit<32> operand_b;   // B operand (right)
    bit<32> result;      // Result of the operation
}

// ****************************************************************************** //
// ************************* S T R U C T U R E S  ******************************* //
// ****************************************************************************** //

// header structure
struct headers {
    eth_mac_t    eth;
    p4calc_t     p4calc;
}

// User metadata structure
struct metadata {
    // empty
    bit<16> tuser_size;
    bit<16> tuser_src;
    bit<16> tuser_dst;
}

// ****************************************************************************** //
// *************************** P A R S E R  ************************************* //
// ****************************************************************************** //

parser MyParser(packet_in packet, 
                out headers hdr, 
                inout metadata meta, 
                inout standard_metadata_t smeta) {
    
    state start {
        packet.extract(hdr.eth);
        transition select(hdr.eth.etherType) {
            P4CALC_ETYPE : parse_p4calc;
            default      : drop;
        }
    }

    state parse_p4calc {
        packet.extract(hdr.p4calc);
        transition select(hdr.p4calc.p, hdr.p4calc.four, hdr.p4calc.version) {
            (P4CALC_P, P4CALC_4, P4CALC_VER) : accept;
            default                          : drop;
        }
    }
    
    state drop {
        smeta.drop = 1;
        transition accept;
    }
}

// ****************************************************************************** //
// **************************  P R O C E S S I N G   **************************** //
// ****************************************************************************** //

control MyProcessing(inout headers hdr, 
                     inout metadata meta, 
                     inout standard_metadata_t smeta) {
    // TODO: Since our P4 logic is a NIC, maybe remove the address switch?  
    action send_back(bit<32> result) {
        bit<48> tmp       = hdr.eth.dstAddr;
        hdr.eth.dstAddr   = hdr.eth.srcAddr;
        hdr.eth.srcAddr   = tmp;
        hdr.p4calc.result = result;
    }
    
    action operation_add() {
        send_back(hdr.p4calc.operand_a + hdr.p4calc.operand_b);
    }
    
    action operation_sub() {
        send_back(hdr.p4calc.operand_a - hdr.p4calc.operand_b);
    }
    
    action operation_mult() {
        send_back(hdr.p4calc.operand_a * hdr.p4calc.operand_b);
    }
    
    action operation_and() {
        send_back(hdr.p4calc.operand_a & hdr.p4calc.operand_b);
    }
    
    action operation_or() {
        send_back(hdr.p4calc.operand_a | hdr.p4calc.operand_b);
    }

    action operation_xor() {
        send_back(hdr.p4calc.operand_a ^ hdr.p4calc.operand_b);
    }

    action operation_drop() {
        smeta.drop = 1;
    }
    
    table calculate {
        key = {
            hdr.p4calc.operation : exact;
        }
        actions = {
            operation_add;
            operation_sub;
            operation_mult;
            operation_and;
            operation_or;
            operation_xor;
            operation_drop;
        }
        direct_match = true;
        default_action = operation_drop();
    }

    apply {
        if (hdr.p4calc.isValid()) {
            calculate.apply();
        } else {
            operation_drop();
        }
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
        packet.emit(hdr.p4calc);
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
