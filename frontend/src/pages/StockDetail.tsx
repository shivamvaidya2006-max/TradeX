import React, { useEffect, useState } from 'react';
import { useParams, useNavigate } from 'react-router-dom';
import {
  ArrowLeft,
  Bookmark,
  Check,
  TrendingUp,
  TrendingDown,
  Activity,
  Layers,
} from 'lucide-react';
import { useMarket } from '../context/MarketContext';
import { stockApi, watchlistApi } from '../services/api';
import { Stock } from '../types';
import { PriceChart } from '../components/PriceChart';
import { OrderBook } from '../components/OrderBook';

export const StockDetail: React.FC = () => {
  const { symbol } = useParams<{ symbol: string }>();
  const navigate = useNavigate();
  const { stockMap, openOrderModal } = useMarket();

  const [stock, setStock] = useState<Stock | null>(null);
  const [inWatchlist, setInWatchlist] = useState<boolean>(false);
  const [watchlistSuccess, setWatchlistSuccess] = useState<string | null>(null);
  const [loading, setLoading] = useState<boolean>(true);

  useEffect(() => {
    if (!symbol) return;

    const loadStock = async () => {
      try {
        setLoading(true);
        // Check live context first
        if (stockMap[symbol]) {
          setStock(stockMap[symbol]);
        } else {
          const res = await stockApi.getBySymbol(symbol);
          if (res.data && res.data.data) {
            setStock(res.data.data);
          }
        }
      } catch (e) {
        console.error('Failed to load stock details', e);
      } finally {
        setLoading(false);
      }
    };

    loadStock();
  }, [symbol, stockMap]);

  if (!stock && loading) {
    return (
      <div className="py-20 text-center text-slate-400 text-sm">
        Loading instrument data...
      </div>
    );
  }

  if (!stock) {
    return (
      <div className="py-20 text-center space-y-3">
        <div className="text-white font-bold text-lg">Stock not found: {symbol}</div>
        <button
          onClick={() => navigate('/markets')}
          className="px-4 py-2 bg-blue-600 text-white rounded-lg text-xs font-semibold"
        >
          Return to Markets
        </button>
      </div>
    );
  }

  const liveStock = stockMap[stock.symbol] || stock;
  const isPos = liveStock.change >= 0;

  const handleToggleWatchlist = async () => {
    try {
      if (inWatchlist) {
        await watchlistApi.removeItem(1, liveStock.id);
        setInWatchlist(false);
        setWatchlistSuccess(`Removed ${liveStock.symbol} from Watchlist`);
      } else {
        await watchlistApi.addItem(1, liveStock.id);
        setInWatchlist(true);
        setWatchlistSuccess(`Added ${liveStock.symbol} to Watchlist`);
      }
      setTimeout(() => setWatchlistSuccess(null), 2500);
    } catch (e) {
      console.error('Watchlist toggle error', e);
    }
  };

  return (
    <div className="space-y-6">
      {/* Top Navigation & Status */}
      <div className="flex items-center justify-between">
        <button
          onClick={() => navigate(-1)}
          className="flex items-center space-x-1.5 text-xs font-medium text-slate-400 hover:text-white transition-colors"
        >
          <ArrowLeft className="w-4 h-4" />
          <span>Back to Markets</span>
        </button>

        {watchlistSuccess && (
          <div className="px-3 py-1 bg-emerald-500/10 border border-emerald-500/20 text-emerald-400 rounded-lg text-xs font-semibold">
            {watchlistSuccess}
          </div>
        )}
      </div>

      {/* Main Stock Banner */}
      <div className="bg-slate-900 border border-slate-800 rounded-2xl p-6 flex flex-wrap items-center justify-between gap-6">
        <div>
          <div className="flex items-center space-x-3">
            <h1 className="text-2xl sm:text-3xl font-black text-white tracking-tight">
              {liveStock.symbol}
            </h1>
            <span className="px-2.5 py-0.5 rounded text-xs font-bold bg-slate-800 text-slate-300 border border-slate-700">
              {liveStock.exchange}
            </span>
            <span className="px-2.5 py-0.5 rounded text-xs font-medium bg-blue-500/10 text-blue-400 border border-blue-500/20">
              {liveStock.sector}
            </span>
          </div>
          <div className="text-sm text-slate-400 mt-1">
            {liveStock.companyName}
          </div>
        </div>

        <div className="flex flex-wrap items-center gap-4 sm:gap-6">
          <div className="text-right">
            <div className="text-2xl sm:text-3xl font-black text-white font-mono">
              ₹{liveStock.currentPrice.toLocaleString('en-IN', { minimumFractionDigits: 2 })}
            </div>
            <div
              className={`text-sm font-semibold font-mono flex items-center justify-end mt-0.5 ${
                isPos ? 'text-emerald-400' : 'text-rose-400'
              }`}
            >
              {isPos ? '+' : ''}
              {liveStock.change.toFixed(2)} ({liveStock.changePercent.toFixed(2)}%)
            </div>
          </div>

          {/* Action Buttons */}
          <div className="flex items-center space-x-2">
            <button
              onClick={() => openOrderModal(liveStock, 'BUY')}
              className="px-5 py-2.5 bg-blue-600 hover:bg-blue-500 text-white rounded-xl text-sm font-bold shadow-lg shadow-blue-600/30 transition-all"
            >
              BUY
            </button>
            <button
              onClick={() => openOrderModal(liveStock, 'SELL')}
              className="px-5 py-2.5 bg-rose-600 hover:bg-rose-500 text-white rounded-xl text-sm font-bold shadow-lg shadow-rose-600/30 transition-all"
            >
              SELL
            </button>
            <button
              onClick={handleToggleWatchlist}
              title="Toggle Watchlist"
              className={`p-2.5 rounded-xl border transition-colors ${
                inWatchlist
                  ? 'bg-amber-500/10 border-amber-500/30 text-amber-400'
                  : 'bg-slate-950 border-slate-800 text-slate-400 hover:text-white hover:border-slate-700'
              }`}
            >
              <Bookmark className="w-5 h-5" />
            </button>
          </div>
        </div>
      </div>

      {/* OHLCV Statistics Bar */}
      <div className="grid grid-cols-2 sm:grid-cols-3 lg:grid-cols-6 gap-3">
        <div className="bg-slate-900 border border-slate-800 rounded-xl p-3">
          <div className="text-[10px] text-slate-500 uppercase font-semibold">Open Price</div>
          <div className="text-sm font-bold text-slate-200 font-mono mt-1">₹{liveStock.openPrice.toFixed(2)}</div>
        </div>
        <div className="bg-slate-900 border border-slate-800 rounded-xl p-3">
          <div className="text-[10px] text-slate-500 uppercase font-semibold">Prev Close</div>
          <div className="text-sm font-bold text-slate-200 font-mono mt-1">₹{liveStock.previousClose.toFixed(2)}</div>
        </div>
        <div className="bg-slate-900 border border-slate-800 rounded-xl p-3">
          <div className="text-[10px] text-slate-500 uppercase font-semibold">Day High</div>
          <div className="text-sm font-bold text-emerald-400 font-mono mt-1">₹{liveStock.dayHigh.toFixed(2)}</div>
        </div>
        <div className="bg-slate-900 border border-slate-800 rounded-xl p-3">
          <div className="text-[10px] text-slate-500 uppercase font-semibold">Day Low</div>
          <div className="text-sm font-bold text-rose-400 font-mono mt-1">₹{liveStock.dayLow.toFixed(2)}</div>
        </div>
        <div className="bg-slate-900 border border-slate-800 rounded-xl p-3">
          <div className="text-[10px] text-slate-500 uppercase font-semibold">Volume</div>
          <div className="text-sm font-bold text-slate-200 font-mono mt-1">{liveStock.volume.toLocaleString('en-IN')}</div>
        </div>
        <div className="bg-slate-900 border border-slate-800 rounded-xl p-3">
          <div className="text-[10px] text-slate-500 uppercase font-semibold">Market Cap</div>
          <div className="text-sm font-bold text-slate-200 font-mono mt-1">₹{(liveStock.marketCap / 10000000).toFixed(0)} Cr</div>
        </div>
      </div>

      {/* Main Grid: Interactive Chart on Left, Order Book on Right */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        <div className="lg:col-span-2">
          <PriceChart
            symbol={liveStock.symbol}
            previousClose={liveStock.previousClose}
            currentPrice={liveStock.currentPrice}
          />
        </div>

        {/* Right Column: Order Book */}
        <div>
          <OrderBook
            symbol={liveStock.symbol}
            onSelectPrice={(price, type) => {
              openOrderModal(liveStock, type);
            }}
          />
        </div>
      </div>
    </div>
  );
};
