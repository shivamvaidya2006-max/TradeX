import React from 'react';
import { TrendingUp, TrendingDown, Wallet as WalletIcon, PieChart, ShieldAlert } from 'lucide-react';
import { PortfolioSummary as SummaryType, Wallet } from '../types';

interface PortfolioSummaryProps {
  summary: SummaryType | null;
  wallet: Wallet | null;
}

export const PortfolioSummary: React.FC<PortfolioSummaryProps> = ({ summary, wallet }) => {
  const totalInvested = summary?.totalInvested ?? 0;
  const totalCurrentValue = summary?.totalCurrentValue ?? 0;
  const totalPnl = summary?.totalPnl ?? 0;
  const totalPnlPercent = summary?.totalPnlPercent ?? 0;
  const todayPnl = summary?.todayPnl ?? 0;
  const todayPnlPercent = summary?.todayPnlPercent ?? 0;

  const isTotalPos = totalPnl >= 0;
  const isTodayPos = todayPnl >= 0;

  return (
    <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-4">
      {/* Total Invested */}
      <div className="bg-slate-900 border border-slate-800 rounded-xl p-4 flex flex-col justify-between">
        <div className="flex items-center justify-between text-slate-400">
          <span className="text-xs font-semibold uppercase tracking-wider">Total Invested</span>
          <PieChart className="w-4 h-4 text-blue-400" />
        </div>
        <div className="mt-3">
          <div className="text-xl lg:text-2xl font-bold text-white tracking-tight">
            ₹{totalInvested.toLocaleString('en-IN', { minimumFractionDigits: 2, maximumFractionDigits: 2 })}
          </div>
          <div className="text-[11px] text-slate-500 mt-0.5">
            Initial Capital Deployed
          </div>
        </div>
      </div>

      {/* Current Value */}
      <div className="bg-slate-900 border border-slate-800 rounded-xl p-4 flex flex-col justify-between">
        <div className="flex items-center justify-between text-slate-400">
          <span className="text-xs font-semibold uppercase tracking-wider">Current Value</span>
          <div className="w-2 h-2 rounded-full bg-blue-500 animate-pulse" />
        </div>
        <div className="mt-3">
          <div className="text-xl lg:text-2xl font-bold text-white tracking-tight">
            ₹{totalCurrentValue.toLocaleString('en-IN', { minimumFractionDigits: 2, maximumFractionDigits: 2 })}
          </div>
          <div className="text-[11px] text-slate-500 mt-0.5">
            Live Equity Valuation
          </div>
        </div>
      </div>

      {/* Total P&L */}
      <div className="bg-slate-900 border border-slate-800 rounded-xl p-4 flex flex-col justify-between">
        <div className="flex items-center justify-between text-slate-400">
          <span className="text-xs font-semibold uppercase tracking-wider">Total P&L</span>
          {isTotalPos ? (
            <TrendingUp className="w-4 h-4 text-emerald-400" />
          ) : (
            <TrendingDown className="w-4 h-4 text-rose-400" />
          )}
        </div>
        <div className="mt-3">
          <div className={`text-xl lg:text-2xl font-bold tracking-tight ${isTotalPos ? 'text-emerald-400' : 'text-rose-400'}`}>
            {isTotalPos ? '+' : ''}₹{totalPnl.toLocaleString('en-IN', { minimumFractionDigits: 2, maximumFractionDigits: 2 })}
          </div>
          <div className={`text-[11px] font-semibold mt-0.5 ${isTotalPos ? 'text-emerald-400/90' : 'text-rose-400/90'}`}>
            {isTotalPos ? '+' : ''}{totalPnlPercent.toFixed(2)}% Overall
          </div>
        </div>
      </div>

      {/* Today's P&L */}
      <div className="bg-slate-900 border border-slate-800 rounded-xl p-4 flex flex-col justify-between">
        <div className="flex items-center justify-between text-slate-400">
          <span className="text-xs font-semibold uppercase tracking-wider">Today's P&L</span>
          {isTodayPos ? (
            <TrendingUp className="w-4 h-4 text-emerald-400" />
          ) : (
            <TrendingDown className="w-4 h-4 text-rose-400" />
          )}
        </div>
        <div className="mt-3">
          <div className={`text-xl lg:text-2xl font-bold tracking-tight ${isTodayPos ? 'text-emerald-400' : 'text-rose-400'}`}>
            {isTodayPos ? '+' : ''}₹{todayPnl.toLocaleString('en-IN', { minimumFractionDigits: 2, maximumFractionDigits: 2 })}
          </div>
          <div className={`text-[11px] font-semibold mt-0.5 ${isTodayPos ? 'text-emerald-400/90' : 'text-rose-400/90'}`}>
            {isTodayPos ? '+' : ''}{todayPnlPercent.toFixed(2)}% Today
          </div>
        </div>
      </div>
    </div>
  );
};
