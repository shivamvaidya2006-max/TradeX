import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { ArrowUpRight, ArrowDownRight, Search, Plus, Minus, ExternalLink } from 'lucide-react';
import { Holding } from '../types';
import { useMarket } from '../context/MarketContext';

interface HoldingsTableProps {
  holdings: Holding[];
  onRefresh?: () => void;
}

export const HoldingsTable: React.FC<HoldingsTableProps> = ({ holdings, onRefresh }) => {
  const navigate = useNavigate();
  const { stockMap, openOrderModal } = useMarket();
  const [searchTerm, setSearchTerm] = useState('');

  const filteredHoldings = holdings.filter(
    (h) =>
      h.symbol.toLowerCase().includes(searchTerm.toLowerCase()) ||
      h.companyName.toLowerCase().includes(searchTerm.toLowerCase())
  );

  return (
    <div className="bg-slate-900 border border-slate-800 rounded-xl overflow-hidden flex flex-col">
      {/* Header bar */}
      <div className="p-4 border-b border-slate-800 flex flex-wrap items-center justify-between gap-3">
        <div className="flex items-center space-x-2">
          <h3 className="text-sm font-bold text-white uppercase tracking-wider">
            Holdings ({holdings.length})
          </h3>
          <span className="text-xs text-slate-400">Equity Delivery</span>
        </div>

        {/* Search */}
        <div className="relative">
          <Search className="w-3.5 h-3.5 absolute left-3 top-1/2 -translate-y-1/2 text-slate-400" />
          <input
            type="text"
            placeholder="Filter holdings..."
            value={searchTerm}
            onChange={(e) => setSearchTerm(e.target.value)}
            className="pl-8 pr-3 py-1.5 bg-slate-950 border border-slate-800 rounded-lg text-xs text-white placeholder-slate-500 focus:outline-none focus:border-blue-500 w-48 sm:w-60"
          />
        </div>
      </div>

      {/* Table */}
      <div className="overflow-x-auto">
        <table className="w-full text-left text-xs">
          <thead className="bg-slate-950/60 text-slate-400 uppercase text-[10px] tracking-wider border-b border-slate-800/80">
            <tr>
              <th className="py-3 px-4 font-semibold">Instrument</th>
              <th className="py-3 px-4 font-semibold text-right">Qty</th>
              <th className="py-3 px-4 font-semibold text-right">Avg. Price</th>
              <th className="py-3 px-4 font-semibold text-right">LTP</th>
              <th className="py-3 px-4 font-semibold text-right">Current Value</th>
              <th className="py-3 px-4 font-semibold text-right">P&L</th>
              <th className="py-3 px-4 font-semibold text-right">Net Chg%</th>
              <th className="py-3 px-4 font-semibold text-center">Actions</th>
            </tr>
          </thead>
          <tbody className="divide-y divide-slate-800/60">
            {filteredHoldings.length === 0 ? (
              <tr>
                <td colSpan={8} className="py-8 text-center text-slate-500">
                  {searchTerm ? 'No holdings match your filter' : 'No holdings in portfolio yet. Explore markets to buy stocks!'}
                </td>
              </tr>
            ) : (
              filteredHoldings.map((h) => {
                const liveStock = stockMap[h.symbol];
                const ltp = liveStock ? liveStock.currentPrice : h.currentPrice;
                const currentValue = h.quantity * ltp;
                const investedValue = h.quantity * h.averagePrice;
                const pnl = currentValue - investedValue;
                const pnlPct = investedValue > 0 ? (pnl / investedValue) * 100 : 0;
                const isPos = pnl >= 0;

                return (
                  <tr key={h.id} className="hover:bg-slate-800/40 transition-colors">
                    {/* Instrument */}
                    <td className="py-3 px-4">
                      <div
                        onClick={() => navigate(`/stocks/${h.symbol}`)}
                        className="cursor-pointer group flex items-center space-x-1"
                      >
                        <div>
                          <div className="font-bold text-white group-hover:text-blue-400 transition-colors flex items-center gap-1">
                            {h.symbol}
                            <ExternalLink className="w-2.5 h-2.5 opacity-0 group-hover:opacity-100 transition-opacity" />
                          </div>
                          <div className="text-[10px] text-slate-400 truncate max-w-[140px]">
                            {h.companyName}
                          </div>
                        </div>
                      </div>
                    </td>

                    {/* Qty */}
                    <td className="py-3 px-4 text-right font-mono font-medium text-slate-200">
                      {h.quantity}
                    </td>

                    {/* Avg. Price */}
                    <td className="py-3 px-4 text-right font-mono text-slate-300">
                      ₹{h.averagePrice.toFixed(2)}
                    </td>

                    {/* LTP */}
                    <td className="py-3 px-4 text-right font-mono font-semibold text-white">
                      ₹{ltp.toFixed(2)}
                    </td>

                    {/* Current Value */}
                    <td className="py-3 px-4 text-right font-mono font-semibold text-slate-200">
                      ₹{currentValue.toLocaleString('en-IN', { minimumFractionDigits: 2, maximumFractionDigits: 2 })}
                    </td>

                    {/* P&L */}
                    <td className="py-3 px-4 text-right font-mono font-bold">
                      <span className={isPos ? 'text-emerald-400' : 'text-rose-400'}>
                        {isPos ? '+' : ''}₹{pnl.toFixed(2)}
                      </span>
                    </td>

                    {/* Net Chg% */}
                    <td className="py-3 px-4 text-right font-mono font-bold">
                      <span className={`inline-flex items-center ${isPos ? 'text-emerald-400' : 'text-rose-400'}`}>
                        {isPos ? <ArrowUpRight className="w-3 h-3 mr-0.5" /> : <ArrowDownRight className="w-3 h-3 mr-0.5" />}
                        {isPos ? '+' : ''}{pnlPct.toFixed(2)}%
                      </span>
                    </td>

                    {/* Actions */}
                    <td className="py-3 px-4 text-center">
                      <div className="flex items-center justify-center space-x-1.5">
                        <button
                          type="button"
                          onClick={() => {
                            if (liveStock) openOrderModal(liveStock, 'BUY');
                          }}
                          title="Add more (Buy)"
                          className="px-2 py-1 bg-blue-500/10 hover:bg-blue-500/20 text-blue-400 rounded text-[11px] font-semibold transition-colors flex items-center gap-0.5"
                        >
                          <Plus className="w-3 h-3" />
                          <span>Add</span>
                        </button>
                        <button
                          type="button"
                          onClick={() => {
                            if (liveStock) openOrderModal(liveStock, 'SELL');
                          }}
                          title="Exit position (Sell)"
                          className="px-2 py-1 bg-rose-500/10 hover:bg-rose-500/20 text-rose-400 rounded text-[11px] font-semibold transition-colors flex items-center gap-0.5"
                        >
                          <Minus className="w-3 h-3" />
                          <span>Exit</span>
                        </button>
                      </div>
                    </td>
                  </tr>
                );
              })
            )}
          </tbody>
        </table>
      </div>
    </div>
  );
};
