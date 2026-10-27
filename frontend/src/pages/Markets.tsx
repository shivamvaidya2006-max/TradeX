import React, { useState, useMemo } from 'react';
import { useNavigate } from 'react-router-dom';
import {
  Search,
  Filter,
  ArrowUpDown,
  ArrowUpRight,
  ArrowDownRight,
  ExternalLink,
  Plus,
  Bookmark,
  Layers,
} from 'lucide-react';
import { useMarket } from '../context/MarketContext';
import { watchlistApi } from '../services/api';
import { Stock } from '../types';

export const Markets: React.FC = () => {
  const navigate = useNavigate();
  const { stocks, priceFlashes, openOrderModal } = useMarket();

  const [searchQuery, setSearchQuery] = useState('');
  const [selectedSector, setSelectedSector] = useState<string>('ALL');
  const [sortBy, setSortBy] = useState<'marketCap' | 'gainers' | 'losers' | 'price' | 'volume'>('marketCap');
  const [watchlistSuccess, setWatchlistSuccess] = useState<string | null>(null);

  // Extract all sectors
  const sectors = useMemo(() => {
    const set = new Set<string>();
    stocks.forEach((s) => {
      if (s.sector) set.add(s.sector);
    });
    return ['ALL', ...Array.from(set).sort()];
  }, [stocks]);

  // Filter and sort stocks
  const filteredStocks = useMemo(() => {
    return stocks
      .filter((s) => {
        const matchesQuery =
          s.symbol.toLowerCase().includes(searchQuery.toLowerCase()) ||
          s.companyName.toLowerCase().includes(searchQuery.toLowerCase());
        const matchesSector = selectedSector === 'ALL' || s.sector === selectedSector;
        return matchesQuery && matchesSector;
      })
      .sort((a, b) => {
        if (sortBy === 'marketCap') return b.marketCap - a.marketCap;
        if (sortBy === 'gainers') return b.changePercent - a.changePercent;
        if (sortBy === 'losers') return a.changePercent - b.changePercent;
        if (sortBy === 'price') return b.currentPrice - a.currentPrice;
        if (sortBy === 'volume') return b.volume - a.volume;
        return 0;
      });
  }, [stocks, searchQuery, selectedSector, sortBy]);

  const handleAddToWatchlist = async (stock: Stock) => {
    try {
      // Default watchlist id = 1
      await watchlistApi.addItem(1, stock.id);
      setWatchlistSuccess(`Added ${stock.symbol} to Watchlist`);
      setTimeout(() => setWatchlistSuccess(null), 2500);
    } catch (e: any) {
      console.error('Failed to add to watchlist', e);
    }
  };

  return (
    <div className="space-y-6">
      {/* Page Title & Stats */}
      <div className="flex flex-wrap items-center justify-between gap-4">
        <div>
          <h1 className="text-2xl font-black text-white tracking-tight">
            Market Watch & Stock Directory
          </h1>
          <p className="text-xs sm:text-sm text-slate-400 mt-1">
            Browse and trade {stocks.length} equities across NSE and BSE with live order execution
          </p>
        </div>

        {watchlistSuccess && (
          <div className="px-3 py-1.5 bg-emerald-500/10 border border-emerald-500/20 text-emerald-400 rounded-xl text-xs font-semibold animate-in fade-in">
            {watchlistSuccess}
          </div>
        )}
      </div>

      {/* Filter and Search Bar */}
      <div className="bg-slate-900 border border-slate-800 rounded-xl p-4 space-y-4">
        <div className="flex flex-wrap items-center justify-between gap-4">
          {/* Search Input */}
          <div className="relative flex-1 min-w-[260px]">
            <Search className="w-4 h-4 absolute left-3.5 top-1/2 -translate-y-1/2 text-slate-400" />
            <input
              type="text"
              placeholder="Search by symbol or company name (e.g., RELIANCE, TCS)..."
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
              className="w-full pl-10 pr-4 py-2 bg-slate-950 border border-slate-800 rounded-xl text-xs text-white placeholder-slate-500 focus:outline-none focus:border-blue-500 transition-colors"
            />
          </div>

          {/* Sort Selector */}
          <div className="flex items-center space-x-2 text-xs">
            <span className="text-slate-400 font-medium flex items-center gap-1">
              <ArrowUpDown className="w-3.5 h-3.5" /> Sort by:
            </span>
            <select
              value={sortBy}
              onChange={(e) => setSortBy(e.target.value as any)}
              className="bg-slate-950 border border-slate-800 rounded-lg px-3 py-2 text-slate-200 focus:outline-none focus:border-blue-500 font-medium"
            >
              <option value="marketCap">Market Cap (High to Low)</option>
              <option value="gainers">Top Gainers (%)</option>
              <option value="losers">Top Losers (%)</option>
              <option value="price">Price (High to Low)</option>
              <option value="volume">Trading Volume</option>
            </select>
          </div>
        </div>

        {/* Sector Chips */}
        <div className="flex items-center gap-1.5 overflow-x-auto pb-1 text-xs">
          <span className="text-slate-500 font-semibold uppercase text-[10px] tracking-wider pr-2">
            Sector:
          </span>
          {sectors.map((sector) => (
            <button
              key={sector}
              onClick={() => setSelectedSector(sector)}
              className={`px-3 py-1 rounded-lg font-medium whitespace-nowrap transition-colors ${
                selectedSector === sector
                  ? 'bg-blue-600 text-white shadow-sm'
                  : 'bg-slate-950 border border-slate-800 text-slate-400 hover:text-white hover:border-slate-700'
              }`}
            >
              {sector}
            </button>
          ))}
        </div>
      </div>

      {/* Stock Cards / Table */}
      <div className="bg-slate-900 border border-slate-800 rounded-xl overflow-hidden">
        <div className="overflow-x-auto">
          <table className="w-full text-left text-xs">
            <thead className="bg-slate-950/70 text-slate-400 uppercase text-[10px] tracking-wider border-b border-slate-800/80">
              <tr>
                <th className="py-3.5 px-4 font-semibold">Instrument</th>
                <th className="py-3.5 px-4 font-semibold">Sector</th>
                <th className="py-3.5 px-4 font-semibold text-right">LTP (₹)</th>
                <th className="py-3.5 px-4 font-semibold text-right">Change (₹)</th>
                <th className="py-3.5 px-4 font-semibold text-right">Change (%)</th>
                <th className="py-3.5 px-4 font-semibold text-right">Day High / Low</th>
                <th className="py-3.5 px-4 font-semibold text-right">Volume</th>
                <th className="py-3.5 px-4 font-semibold text-right">Market Cap</th>
                <th className="py-3.5 px-4 font-semibold text-center">Trade</th>
              </tr>
            </thead>
            <tbody className="divide-y divide-slate-800/60">
              {filteredStocks.length === 0 ? (
                <tr>
                  <td colSpan={9} className="py-12 text-center text-slate-500">
                    No stocks match your search criteria.
                  </td>
                </tr>
              ) : (
                filteredStocks.map((stk) => {
                  const isPos = stk.change >= 0;
                  const flash = priceFlashes[stk.symbol];
                  const flashClass = flash === 'up' ? 'flash-green' : flash === 'down' ? 'flash-red' : '';

                  return (
                    <tr
                      key={stk.id || stk.symbol}
                      className={`hover:bg-slate-800/40 transition-colors ${flashClass}`}
                    >
                      {/* Instrument */}
                      <td className="py-3.5 px-4">
                        <div
                          onClick={() => navigate(`/stocks/${stk.symbol}`)}
                          className="cursor-pointer group flex items-center space-x-2"
                        >
                          <div>
                            <div className="font-bold text-white group-hover:text-blue-400 transition-colors flex items-center gap-1.5">
                              {stk.symbol}
                              <span className="text-[10px] text-slate-400 font-normal">
                                {stk.exchange}
                              </span>
                              <ExternalLink className="w-2.5 h-2.5 opacity-0 group-hover:opacity-100 transition-opacity text-blue-400" />
                            </div>
                            <div className="text-[11px] text-slate-400 truncate max-w-[150px]">
                              {stk.companyName}
                            </div>
                          </div>
                        </div>
                      </td>

                      {/* Sector */}
                      <td className="py-3.5 px-4">
                        <span className="px-2 py-0.5 rounded text-[10px] font-medium bg-slate-800 text-slate-300 border border-slate-700/60">
                          {stk.sector}
                        </span>
                      </td>

                      {/* LTP */}
                      <td className="py-3.5 px-4 text-right font-mono font-bold text-white text-sm">
                        ₹{stk.currentPrice.toLocaleString('en-IN', { minimumFractionDigits: 2 })}
                      </td>

                      {/* Change */}
                      <td className="py-3.5 px-4 text-right font-mono font-semibold">
                        <span className={isPos ? 'text-emerald-400' : 'text-rose-400'}>
                          {isPos ? '+' : ''}{stk.change.toFixed(2)}
                        </span>
                      </td>

                      {/* Change % */}
                      <td className="py-3.5 px-4 text-right font-mono font-semibold">
                        <span className={`inline-flex items-center ${isPos ? 'text-emerald-400' : 'text-rose-400'}`}>
                          {isPos ? <ArrowUpRight className="w-3 h-3 mr-0.5" /> : <ArrowDownRight className="w-3 h-3 mr-0.5" />}
                          {isPos ? '+' : ''}{stk.changePercent.toFixed(2)}%
                        </span>
                      </td>

                      {/* High / Low */}
                      <td className="py-3.5 px-4 text-right font-mono text-[11px] text-slate-400">
                        <div>H: ₹{stk.dayHigh.toFixed(2)}</div>
                        <div>L: ₹{stk.dayLow.toFixed(2)}</div>
                      </td>

                      {/* Volume */}
                      <td className="py-3.5 px-4 text-right font-mono text-slate-300">
                        {stk.volume.toLocaleString('en-IN')}
                      </td>

                      {/* Market Cap */}
                      <td className="py-3.5 px-4 text-right font-mono text-slate-300">
                        ₹{(stk.marketCap / 10000000).toFixed(0)} Cr
                      </td>

                      {/* Trade Actions */}
                      <td className="py-3.5 px-4 text-center">
                        <div className="flex items-center justify-center space-x-1.5">
                          <button
                            type="button"
                            onClick={() => openOrderModal(stk, 'BUY')}
                            className="px-2.5 py-1 bg-blue-600 hover:bg-blue-500 text-white rounded text-xs font-bold shadow transition-colors"
                          >
                            B
                          </button>
                          <button
                            type="button"
                            onClick={() => openOrderModal(stk, 'SELL')}
                            className="px-2.5 py-1 bg-rose-600 hover:bg-rose-500 text-white rounded text-xs font-bold shadow transition-colors"
                          >
                            S
                          </button>
                          <button
                            type="button"
                            onClick={() => handleAddToWatchlist(stk)}
                            title="Add to Watchlist"
                            className="p-1 text-slate-400 hover:text-amber-400 hover:bg-slate-800 rounded transition-colors"
                          >
                            <Bookmark className="w-3.5 h-3.5" />
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
    </div>
  );
};
