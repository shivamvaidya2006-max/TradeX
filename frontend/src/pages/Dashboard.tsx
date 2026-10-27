import React, { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import {
  TrendingUp,
  TrendingDown,
  Activity,
  ArrowUpRight,
  ArrowDownRight,
  Eye,
  Plus,
  RefreshCw,
  Clock,
  Sparkles,
} from 'lucide-react';
import { useMarket } from '../context/MarketContext';
import { useAuth } from '../context/AuthContext';
import { portfolioApi, walletApi, stockApi } from '../services/api';
import { PortfolioSummary as SummaryType, Wallet, Holding, Stock } from '../types';
import { PortfolioSummary } from '../components/PortfolioSummary';
import { PriceChart } from '../components/PriceChart';
import { OrderBook } from '../components/OrderBook';

export const Dashboard: React.FC = () => {
  const navigate = useNavigate();
  const { user } = useAuth();
  const { stocks, marketOverview, marketOpen, openOrderModal } = useMarket();

  const [portfolioSummary, setPortfolioSummary] = useState<SummaryType | null>(null);
  const [wallet, setWallet] = useState<Wallet | null>(null);
  const [holdings, setHoldings] = useState<Holding[]>([]);
  const [selectedStock, setSelectedStock] = useState<Stock | null>(null);
  const [loading, setLoading] = useState<boolean>(true);
  const [activeTab, setActiveTab] = useState<'gainers' | 'losers' | 'active'>('gainers');

  const fetchDashboardData = async () => {
    try {
      const [portRes, walletRes] = await Promise.all([
        portfolioApi.getPortfolio(),
        walletApi.getWallet(),
      ]);

      if (portRes.data && portRes.data.data) {
        setPortfolioSummary(portRes.data.data.summary);
        setHoldings(portRes.data.data.holdings || []);
      }

      if (walletRes.data && walletRes.data.data) {
        setWallet(walletRes.data.data);
      }
    } catch (e) {
      console.error('Failed to load portfolio/wallet in dashboard', e);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchDashboardData();
  }, []);

  useEffect(() => {
    if (stocks.length > 0 && !selectedStock) {
      // Default to RELIANCE or the first stock
      const def = stocks.find((s) => s.symbol === 'RELIANCE') || stocks[0];
      setSelectedStock(def);
    }
  }, [stocks, selectedStock]);

  const topGainers = marketOverview?.topGainers || stocks.slice(0, 5);
  const topLosers = marketOverview?.topLosers || stocks.slice(-5);
  const mostActive = marketOverview?.mostActive || stocks.slice(0, 5);

  const displayList =
    activeTab === 'gainers' ? topGainers : activeTab === 'losers' ? topLosers : mostActive;

  return (
    <div className="space-y-6">
      {/* Welcome Banner & Market Status */}
      <div className="bg-gradient-to-r from-slate-900 via-slate-900 to-blue-950/40 border border-slate-800 rounded-2xl p-5 flex flex-wrap items-center justify-between gap-4">
        <div>
          <div className="flex items-center space-x-2">
            <h1 className="text-xl sm:text-2xl font-black text-white tracking-tight">
              Good day, {user?.fullName || user?.username || 'Trader'} 👋
            </h1>
            <span className="px-2 py-0.5 rounded text-[10px] font-bold bg-blue-500/20 text-blue-400 border border-blue-500/30">
              EDUCATIONAL SIMULATION
            </span>
          </div>
          <p className="text-xs sm:text-sm text-slate-400 mt-1">
            Real-time simulated NSE & BSE trading with live quotes and instant order execution
          </p>
        </div>

        <div className="flex items-center space-x-3">
          <div className="flex items-center space-x-2 bg-slate-950/80 px-3.5 py-2 rounded-xl border border-slate-800 text-xs">
            <div className={`w-2.5 h-2.5 rounded-full ${marketOpen ? 'bg-emerald-500 animate-pulse' : 'bg-rose-500'}`} />
            <div>
              <div className="font-bold text-white leading-none">
                {marketOpen ? 'NSE / BSE LIVE' : 'MARKET CLOSED'}
              </div>
              <div className="text-[10px] text-slate-400 mt-0.5">
                09:15 - 15:30 IST
              </div>
            </div>
          </div>
        </div>
      </div>

      {/* Portfolio Financial Metrics Cards */}
      <PortfolioSummary summary={portfolioSummary} wallet={wallet} />

      {/* Main Grid: Interactive Chart & Order Depth on Left, Market Movers on Right */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        {/* Left Column (2 cols): Selected Stock Chart + Live Order Depth */}
        <div className="lg:col-span-2 space-y-6">
          {selectedStock && (
            <div className="space-y-4">
              {/* Selected Stock Banner */}
              <div className="bg-slate-900 border border-slate-800 rounded-xl p-4 flex flex-wrap items-center justify-between gap-3">
                <div className="flex items-center space-x-3">
                  <div>
                    <div className="flex items-center space-x-2">
                      <h2 className="text-lg font-bold text-white tracking-tight">
                        {selectedStock.symbol}
                      </h2>
                      <span className="text-xs px-2 py-0.5 rounded bg-slate-800 text-slate-400">
                        {selectedStock.exchange}
                      </span>
                      <span className="text-xs px-2 py-0.5 rounded bg-slate-800 text-slate-400">
                        {selectedStock.sector}
                      </span>
                    </div>
                    <div className="text-xs text-slate-400 truncate max-w-[260px]">
                      {selectedStock.companyName}
                    </div>
                  </div>
                </div>

                <div className="flex items-center space-x-4">
                  <div className="text-right">
                    <div className="text-lg font-bold text-white font-mono">
                      ₹{selectedStock.currentPrice.toFixed(2)}
                    </div>
                    <div
                      className={`text-xs font-semibold font-mono flex items-center justify-end ${
                        selectedStock.change >= 0 ? 'text-emerald-400' : 'text-rose-400'
                      }`}
                    >
                      {selectedStock.change >= 0 ? '+' : ''}
                      {selectedStock.change.toFixed(2)} ({selectedStock.changePercent.toFixed(2)}%)
                    </div>
                  </div>

                  <div className="flex items-center space-x-1.5">
                    <button
                      onClick={() => openOrderModal(selectedStock, 'BUY')}
                      className="px-3.5 py-1.5 bg-blue-600 hover:bg-blue-500 text-white rounded-lg text-xs font-bold shadow-md shadow-blue-600/30 transition-all"
                    >
                      BUY
                    </button>
                    <button
                      onClick={() => openOrderModal(selectedStock, 'SELL')}
                      className="px-3.5 py-1.5 bg-rose-600 hover:bg-rose-500 text-white rounded-lg text-xs font-bold shadow-md shadow-rose-600/30 transition-all"
                    >
                      SELL
                    </button>
                  </div>
                </div>
              </div>

              {/* Chart */}
              <PriceChart
                symbol={selectedStock.symbol}
                previousClose={selectedStock.previousClose}
                currentPrice={selectedStock.currentPrice}
              />

              {/* Order Book Depth */}
              <OrderBook
                symbol={selectedStock.symbol}
                onSelectPrice={(price, type) => {
                  openOrderModal(selectedStock, type);
                }}
              />
            </div>
          )}
        </div>

        {/* Right Column (1 col): Market Movers & Mini Watchlist */}
        <div className="space-y-6">
          {/* Movers Card */}
          <div className="bg-slate-900 border border-slate-800 rounded-xl overflow-hidden flex flex-col">
            <div className="p-4 border-b border-slate-800 flex items-center justify-between">
              <div className="flex items-center space-x-2">
                <Activity className="w-4 h-4 text-blue-400" />
                <h3 className="text-xs font-bold text-white uppercase tracking-wider">
                  Market Movers
                </h3>
              </div>
              <button
                onClick={() => navigate('/markets')}
                className="text-xs text-blue-400 hover:text-blue-300 font-medium"
              >
                View All
              </button>
            </div>

            {/* Tab switch */}
            <div className="grid grid-cols-3 bg-slate-950 p-1 border-b border-slate-800 text-xs font-medium">
              <button
                onClick={() => setActiveTab('gainers')}
                className={`py-1.5 rounded-lg transition-colors ${
                  activeTab === 'gainers'
                    ? 'bg-slate-800 text-emerald-400 font-bold'
                    : 'text-slate-400 hover:text-white'
                }`}
              >
                Gainers
              </button>
              <button
                onClick={() => setActiveTab('losers')}
                className={`py-1.5 rounded-lg transition-colors ${
                  activeTab === 'losers'
                    ? 'bg-slate-800 text-rose-400 font-bold'
                    : 'text-slate-400 hover:text-white'
                }`}
              >
                Losers
              </button>
              <button
                onClick={() => setActiveTab('active')}
                className={`py-1.5 rounded-lg transition-colors ${
                  activeTab === 'active'
                    ? 'bg-slate-800 text-blue-400 font-bold'
                    : 'text-slate-400 hover:text-white'
                }`}
              >
                Active
              </button>
            </div>

            {/* List */}
            <div className="divide-y divide-slate-800/60 overflow-y-auto max-h-[380px]">
              {displayList.map((stk) => {
                const isPos = stk.change >= 0;
                return (
                  <div
                    key={stk.id || stk.symbol}
                    onClick={() => setSelectedStock(stk)}
                    className={`p-3.5 flex items-center justify-between cursor-pointer transition-colors hover:bg-slate-800/50 ${
                      selectedStock?.symbol === stk.symbol ? 'bg-blue-600/10 border-l-2 border-blue-500' : ''
                    }`}
                  >
                    <div>
                      <div className="font-bold text-xs text-white flex items-center gap-1.5">
                        {stk.symbol}
                        <span className="text-[10px] text-slate-500 font-normal">
                          {stk.exchange}
                        </span>
                      </div>
                      <div className="text-[11px] text-slate-400 truncate max-w-[130px]">
                        {stk.companyName}
                      </div>
                    </div>

                    <div className="flex items-center space-x-3">
                      <div className="text-right">
                        <div className="text-xs font-mono font-bold text-white">
                          ₹{stk.currentPrice.toFixed(2)}
                        </div>
                        <div
                          className={`text-[10px] font-mono font-semibold flex items-center justify-end ${
                            isPos ? 'text-emerald-400' : 'text-rose-400'
                          }`}
                        >
                          {isPos ? <ArrowUpRight className="w-2.5 h-2.5 mr-0.5" /> : <ArrowDownRight className="w-2.5 h-2.5 mr-0.5" />}
                          {isPos ? '+' : ''}{stk.changePercent.toFixed(2)}%
                        </div>
                      </div>

                      <div className="flex items-center space-x-1" onClick={(e) => e.stopPropagation()}>
                        <button
                          onClick={() => openOrderModal(stk, 'BUY')}
                          className="px-2 py-1 bg-blue-500/10 hover:bg-blue-500/20 text-blue-400 rounded text-[10px] font-bold"
                        >
                          B
                        </button>
                        <button
                          onClick={() => openOrderModal(stk, 'SELL')}
                          className="px-2 py-1 bg-rose-500/10 hover:bg-rose-500/20 text-rose-400 rounded text-[10px] font-bold"
                        >
                          S
                        </button>
                      </div>
                    </div>
                  </div>
                );
              })}
            </div>
          </div>

          {/* Quick Holdings Preview */}
          <div className="bg-slate-900 border border-slate-800 rounded-xl p-4 space-y-3">
            <div className="flex items-center justify-between">
              <h3 className="text-xs font-bold text-white uppercase tracking-wider">
                My Holdings ({holdings.length})
              </h3>
              <button
                onClick={() => navigate('/portfolio')}
                className="text-xs text-blue-400 hover:text-blue-300 font-medium"
              >
                Go to Portfolio
              </button>
            </div>

            {holdings.length === 0 ? (
              <div className="text-xs text-slate-500 text-center py-4">
                No active holdings. Place a buy order to build your portfolio.
              </div>
            ) : (
              <div className="space-y-2">
                {holdings.slice(0, 4).map((h) => {
                  const pnl = (h.currentPrice - h.averagePrice) * h.quantity;
                  const isPos = pnl >= 0;
                  return (
                    <div
                      key={h.id}
                      className="p-2.5 bg-slate-950/70 border border-slate-800/80 rounded-lg flex items-center justify-between text-xs"
                    >
                      <div>
                        <div className="font-bold text-white">{h.symbol}</div>
                        <div className="text-[10px] text-slate-400">{h.quantity} shares @ ₹{h.averagePrice.toFixed(1)}</div>
                      </div>
                      <div className="text-right">
                        <div className="font-mono font-semibold text-slate-200">
                          ₹{(h.quantity * h.currentPrice).toFixed(2)}
                        </div>
                        <div className={`text-[10px] font-mono font-bold ${isPos ? 'text-emerald-400' : 'text-rose-400'}`}>
                          {isPos ? '+' : ''}₹{pnl.toFixed(2)}
                        </div>
                      </div>
                    </div>
                  );
                })}
              </div>
            )}
          </div>
        </div>
      </div>
    </div>
  );
};
