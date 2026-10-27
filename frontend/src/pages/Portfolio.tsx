import React, { useState, useEffect } from 'react';
import { RefreshCw, Briefcase, Layers, PieChart } from 'lucide-react';
import { portfolioApi, walletApi } from '../services/api';
import { Holding, Position, PortfolioSummary as SummaryType, Wallet } from '../types';
import { PortfolioSummary } from '../components/PortfolioSummary';
import { HoldingsTable } from '../components/HoldingsTable';
import { PositionsTable } from '../components/PositionsTable';

export const Portfolio: React.FC = () => {
  const [activeTab, setActiveTab] = useState<'holdings' | 'positions'>('holdings');
  const [holdings, setHoldings] = useState<Holding[]>([]);
  const [positions, setPositions] = useState<Position[]>([]);
  const [summary, setSummary] = useState<SummaryType | null>(null);
  const [wallet, setWallet] = useState<Wallet | null>(null);
  const [loading, setLoading] = useState<boolean>(true);

  const loadPortfolio = async () => {
    try {
      setLoading(true);
      const [portRes, walletRes] = await Promise.all([
        portfolioApi.getPortfolio(),
        walletApi.getWallet(),
      ]);

      if (portRes.data && portRes.data.data) {
        setHoldings(portRes.data.data.holdings || []);
        setPositions(portRes.data.data.positions || []);
        setSummary(portRes.data.data.summary);
      }

      if (walletRes.data && walletRes.data.data) {
        setWallet(walletRes.data.data);
      }
    } catch (e) {
      console.error('Failed to load portfolio', e);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    loadPortfolio();
  }, []);

  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="flex flex-wrap items-center justify-between gap-4">
        <div>
          <h1 className="text-2xl font-black text-white tracking-tight">
            Portfolio & Investments
          </h1>
          <p className="text-xs sm:text-sm text-slate-400 mt-1">
            Real-time equity valuation, realized/unrealized profit & loss calculations
          </p>
        </div>

        <button
          onClick={loadPortfolio}
          className="p-2 bg-slate-900 hover:bg-slate-800 border border-slate-800 text-slate-300 rounded-xl transition-colors"
          title="Refresh Portfolio"
        >
          <RefreshCw className={`w-4 h-4 ${loading ? 'animate-spin text-blue-500' : ''}`} />
        </button>
      </div>

      {/* Summary Cards */}
      <PortfolioSummary summary={summary} wallet={wallet} />

      {/* Tabs */}
      <div className="flex items-center space-x-2 border-b border-slate-800 pb-2">
        <button
          onClick={() => setActiveTab('holdings')}
          className={`px-4 py-2 rounded-xl text-xs font-bold transition-all flex items-center space-x-2 ${
            activeTab === 'holdings'
              ? 'bg-blue-600 text-white shadow-md shadow-blue-600/30'
              : 'bg-slate-900 text-slate-400 hover:text-white border border-slate-800'
          }`}
        >
          <Briefcase className="w-3.5 h-3.5" />
          <span>Holdings ({holdings.length})</span>
        </button>

        <button
          onClick={() => setActiveTab('positions')}
          className={`px-4 py-2 rounded-xl text-xs font-bold transition-all flex items-center space-x-2 ${
            activeTab === 'positions'
              ? 'bg-blue-600 text-white shadow-md shadow-blue-600/30'
              : 'bg-slate-900 text-slate-400 hover:text-white border border-slate-800'
          }`}
        >
          <Layers className="w-3.5 h-3.5" />
          <span>Positions ({positions.length})</span>
        </button>
      </div>

      {/* Tables based on active tab */}
      {activeTab === 'holdings' ? (
        <HoldingsTable holdings={holdings} onRefresh={loadPortfolio} />
      ) : (
        <PositionsTable positions={positions} onRefresh={loadPortfolio} />
      )}
    </div>
  );
};
