/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef __DEBUGGER_RIVERLIB_CACHE_TAGMEMNWAY_H__
#define __DEBUGGER_RIVERLIB_CACHE_TAGMEMNWAY_H__

#include <systemc.h>
#include "riscv-isa.h"
#include "../river_cfg.h"
#include "tagwaymem.h"
#include "lrunway.h"

namespace debugger {

SC_MODULE(TagMemNWay) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<bool> i_cs;
    sc_in<bool> i_flush;
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_addr;
    sc_in<sc_biguint<4*BUS_DATA_WIDTH>> i_wdata;
    sc_in<sc_uint<4*BUS_DATA_BYTES>> i_wstrb;
    sc_in<sc_uint<DTAG_FL_TOTAL>> i_wflags;
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_raddr;
    sc_out<sc_biguint<4*BUS_DATA_WIDTH>> o_rdata;
    sc_out<sc_uint<DTAG_FL_TOTAL>> o_rflags;
    sc_out<bool> o_rvalid;
    sc_out<bool> o_hit;

    void comb();
    void registers();

    SC_HAS_PROCESS(TagMemNWay);

    TagMemNWay(sc_module_name name_, bool async_reset);
    virtual ~TagMemNWay();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    struct WayInType {
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> addr;
        sc_signal<sc_uint<4*BUS_DATA_BYTES>> wstrb;
        sc_signal<sc_biguint<4*BUS_DATA_WIDTH>> wdata;
        sc_signal<sc_uint<DTAG_FL_TOTAL>> wflags;
        sc_signal<bool> valid;
    };
   struct WayOutType {
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> raddr;
        sc_signal<sc_biguint<4*BUS_DATA_WIDTH>> rdata;
        sc_signal<sc_uint<DTAG_FL_TOTAL>> rflags;
        sc_signal<bool> valid;
        sc_signal<bool> hit;
    };

    TagWayMem<BUS_ADDR_WIDTH, CFG_DLOG2_LINES_PER_WAY, 4*BUS_DATA_BYTES,
              DTAG_SIZE, DTAG_FL_TOTAL> *wayx[DCACHE_WAYS];
    lrunway<CFG_DLOG2_LINES_PER_WAY, CFG_DLOG2_NWAYS> *lru;

    WayInType way_i[DCACHE_WAYS];
    WayOutType way_o[DCACHE_WAYS];

    sc_signal<bool> lrui_flush;
    sc_signal<sc_uint<CFG_DLOG2_LINES_PER_WAY>> lrui_addr;
    sc_signal<bool> lrui_we;
    sc_signal<sc_uint<2>> lrui_lru;
    sc_signal<sc_uint<2>> lruo_lru;

    sc_uint<BUS_ADDR_WIDTH> mux_raddr;
    sc_biguint<4*BUS_DATA_WIDTH> mux_rdata;
    sc_uint<DTAG_FL_TOTAL> mux_rflags;
    bool mux_valid;

    struct RegistersType {
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> req_addr;
        sc_signal<bool> re;
    } v, r;

    void R_RESET(RegistersType &iv) {
        iv.req_addr = 0;
        iv.re = 0;
    }

    bool async_reset_;
};

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_CACHE_TAGMEMNWAY_H__