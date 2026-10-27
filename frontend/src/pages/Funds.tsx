import React, { useState, useEffect } from 'react';
import {
  Wallet as WalletIcon,
  ArrowDownCircle,
  ArrowUpCircle,
  RefreshCw,
  AlertCircle,
  CheckCircle2,
  DollarSign,
  ShieldCheck,
  History,
} from 'lucide-react';
import { walletApi } from '../services/api';
import { Wallet, Transaction } from '../types';

export const Funds: React.FC = () => {
  const [wallet, setWallet] = useState<Wallet | null>(null);
  const [transactions, setTransactions] = useState<Transaction[]>([]);
  const [depositAmount, setDepositAmount] = useState<number>(50000);
  const [withdrawAmount, setWithdrawAmount] = useState<number>(10000);
  const [showDepositModal, setShowDepositModal] = useState<boolean>(false);
  const [showWithdrawModal, setShowWithdrawModal] = useState<boolean>(false);
  const [loading, setLoading] = useState<boolean>(true);
  const [submitting, setSubmitting] = useState<boolean>(false);
  const [feedback, setFeedback] = useState<{ type: 'success' | 'error'; message: string } | null>(null);

  const fetchFundsData = async () => {
    try {
      setLoading(true);
      const [wRes, txRes] = await Promise.all([
        walletApi.getWallet(),
        walletApi.getTransactions(),
      ]);

      if (wRes.data && wRes.data.data) {
        setWallet(wRes.data.data);
      }
      if (txRes.data && txRes.data.data) {
        setTransactions(txRes.data.data);
      }
    } catch (e) {
      console.error('Failed to load wallet data', e);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchFundsData();
  }, []);

  const handleDeposit = async (e: React.FormEvent) => {
    e.preventDefault();
    if (depositAmount <= 0) return;

    try {
      setSubmitting(true);
      await walletApi.deposit(depositAmount);
      setFeedback({
        type: 'success',
        message: `Successfully deposited ₹${depositAmount.toLocaleString('en-IN')} into your paper-trading wallet!`,
      });
      setShowDepositModal(false);
      fetchFundsData();
    } catch (err: any) {
      setFeedback({
        type: 'error',
        message: err.response?.data?.error || 'Deposit failed',
      });
    } finally {
      setSubmitting(false);
      setTimeout(() => setFeedback(null), 4000);
    }
  };

  const handleWithdraw = async (e: React.FormEvent) => {
    e.preventDefault();
    if (withdrawAmount <= 0) return;

    if (wallet && withdrawAmount > wallet.withdrawableAmount) {
      setFeedback({
        type: 'error',
        message: `Amount exceeds withdrawable balance of ₹${wallet.withdrawableAmount.toLocaleString('en-IN')}`,
      });
      return;
    }

    try {
      setSubmitting(true);
      await walletApi.withdraw(withdrawAmount);
      setFeedback({
        type: 'success',
        message: `Successfully simulated withdrawal of ₹${withdrawAmount.toLocaleString('en-IN')}`,
      });
      setShowWithdrawModal(false);
      fetchFundsData();
    } catch (err: any) {
      setFeedback({
        type: 'error',
        message: err.response?.data?.error || 'Withdrawal failed',
      });
    } finally {
      setSubmitting(false);
      setTimeout(() => setFeedback(null), 4000);
    }
  };

  const available = wallet?.availableBalance || 0;
  const invested = wallet?.investedAmount || 0;
  const withdrawable = wallet?.withdrawableAmount || 0;
  const totalAccountValue = available + invested;

  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="flex flex-wrap items-center justify-between gap-4">
        <div>
          <h1 className="text-2xl font-black text-white tracking-tight">
            Funds & Virtual Wallet
          </h1>
          <p className="text-xs sm:text-sm text-slate-400 mt-1">
            Manage your simulated paper-trading margin and virtual bank transfers
          </p>
        </div>

        <div className="flex items-center space-x-3">
          <button
            onClick={() => setShowDepositModal(true)}
            className="px-4 py-2 bg-blue-600 hover:bg-blue-500 text-white rounded-xl text-xs font-bold shadow-lg shadow-blue-600/30 transition-all flex items-center space-x-1.5"
          >
            <ArrowDownCircle className="w-4 h-4" />
            <span>Add Funds (Virtual)</span>
          </button>

          <button
            onClick={() => setShowWithdrawModal(true)}
            className="px-4 py-2 bg-slate-900 hover:bg-slate-800 border border-slate-700 text-slate-200 rounded-xl text-xs font-bold transition-all flex items-center space-x-1.5"
          >
            <ArrowUpCircle className="w-4 h-4" />
            <span>Withdraw</span>
          </button>
        </div>
      </div>

      {/* Feedback banner */}
      {feedback && (
        <div
          className={`p-3.5 rounded-xl border flex items-center space-x-2 text-xs font-semibold ${
            feedback.type === 'success'
              ? 'bg-emerald-500/10 border-emerald-500/20 text-emerald-400'
              : 'bg-rose-500/10 border-rose-500/20 text-rose-400'
          }`}
        >
          {feedback.type === 'success' ? (
            <CheckCircle2 className="w-4 h-4 shrink-0" />
          ) : (
            <AlertCircle className="w-4 h-4 shrink-0" />
          )}
          <span>{feedback.message}</span>
        </div>
      )}

      {/* Fund Metrics Cards */}
      <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-4">
        {/* Available Balance */}
        <div className="bg-slate-900 border border-slate-800 rounded-xl p-5 flex flex-col justify-between">
          <div className="flex items-center justify-between text-slate-400">
            <span className="text-xs font-semibold uppercase tracking-wider">Available Cash</span>
            <WalletIcon className="w-4 h-4 text-emerald-400" />
          </div>
          <div className="mt-3">
            <div className="text-2xl font-black text-white font-mono">
              ₹{available.toLocaleString('en-IN', { minimumFractionDigits: 2, maximumFractionDigits: 2 })}
            </div>
            <div className="text-[11px] text-slate-500 mt-0.5">
              Instant Margin for Trading
            </div>
          </div>
        </div>

        {/* Invested Amount */}
        <div className="bg-slate-900 border border-slate-800 rounded-xl p-5 flex flex-col justify-between">
          <div className="flex items-center justify-between text-slate-400">
            <span className="text-xs font-semibold uppercase tracking-wider">Capital Utilized</span>
            <DollarSign className="w-4 h-4 text-blue-400" />
          </div>
          <div className="mt-3">
            <div className="text-2xl font-black text-white font-mono">
              ₹{invested.toLocaleString('en-IN', { minimumFractionDigits: 2, maximumFractionDigits: 2 })}
            </div>
            <div className="text-[11px] text-slate-500 mt-0.5">
              Locked in Equity Holdings
            </div>
          </div>
        </div>

        {/* Withdrawable Balance */}
        <div className="bg-slate-900 border border-slate-800 rounded-xl p-5 flex flex-col justify-between">
          <div className="flex items-center justify-between text-slate-400">
            <span className="text-xs font-semibold uppercase tracking-wider">Withdrawable</span>
            <ArrowUpCircle className="w-4 h-4 text-amber-400" />
          </div>
          <div className="mt-3">
            <div className="text-2xl font-black text-white font-mono">
              ₹{withdrawable.toLocaleString('en-IN', { minimumFractionDigits: 2, maximumFractionDigits: 2 })}
            </div>
            <div className="text-[11px] text-slate-500 mt-0.5">
              Settled Funds Available
            </div>
          </div>
        </div>

        {/* Total Account Valuation */}
        <div className="bg-slate-900 border border-slate-800 rounded-xl p-5 flex flex-col justify-between">
          <div className="flex items-center justify-between text-slate-400">
            <span className="text-xs font-semibold uppercase tracking-wider">Total Net Worth</span>
            <ShieldCheck className="w-4 h-4 text-indigo-400" />
          </div>
          <div className="mt-3">
            <div className="text-2xl font-black text-white font-mono">
              ₹{totalAccountValue.toLocaleString('en-IN', { minimumFractionDigits: 2, maximumFractionDigits: 2 })}
            </div>
            <div className="text-[11px] text-slate-500 mt-0.5">
              Cash + Portfolio Value
            </div>
          </div>
        </div>
      </div>

      {/* Wallet Ledger / Transactions */}
      <div className="bg-slate-900 border border-slate-800 rounded-xl overflow-hidden">
        <div className="p-4 border-b border-slate-800 flex items-center justify-between">
          <div className="flex items-center space-x-2">
            <History className="w-4 h-4 text-blue-400" />
            <h3 className="text-xs font-bold text-white uppercase tracking-wider">
              Virtual Funds Transaction History
            </h3>
          </div>
          <span className="text-xs text-slate-400">
            {transactions.length} records
          </span>
        </div>

        <div className="overflow-x-auto">
          <table className="w-full text-left text-xs">
            <thead className="bg-slate-950/70 text-slate-400 uppercase text-[10px] tracking-wider border-b border-slate-800/80">
              <tr>
                <th className="py-3 px-4 font-semibold">Timestamp</th>
                <th className="py-3 px-4 font-semibold">Type</th>
                <th className="py-3 px-4 font-semibold">Description</th>
                <th className="py-3 px-4 font-semibold text-right">Amount</th>
              </tr>
            </thead>
            <tbody className="divide-y divide-slate-800/60">
              {transactions.length === 0 ? (
                <tr>
                  <td colSpan={4} className="py-10 text-center text-slate-500">
                    No transactions recorded yet.
                  </td>
                </tr>
              ) : (
                transactions.map((tx) => {
                  const isCredit = tx.type === 'DEPOSIT' || tx.type === 'SELL';
                  return (
                    <tr key={tx.id} className="hover:bg-slate-800/40 transition-colors">
                      <td className="py-3 px-4 text-slate-400 font-mono text-[11px]">
                        {tx.createdAt}
                      </td>
                      <td className="py-3 px-4">
                        <span
                          className={`px-2 py-0.5 rounded text-[10px] font-bold ${
                            isCredit
                              ? 'bg-emerald-500/10 text-emerald-400 border border-emerald-500/20'
                              : 'bg-rose-500/10 text-rose-400 border border-rose-500/20'
                          }`}
                        >
                          {tx.type}
                        </span>
                      </td>
                      <td className="py-3 px-4 text-slate-200">
                        {tx.description}
                      </td>
                      <td className="py-3 px-4 text-right font-mono font-bold">
                        <span className={isCredit ? 'text-emerald-400' : 'text-rose-400'}>
                          {isCredit ? '+' : '-'}₹{tx.amount.toLocaleString('en-IN', { minimumFractionDigits: 2 })}
                        </span>
                      </td>
                    </tr>
                  );
                })
              )}
            </tbody>
          </table>
        </div>
      </div>

      {/* Add Funds Modal */}
      {showDepositModal && (
        <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-slate-950/80 backdrop-blur-xs">
          <div className="bg-slate-900 border border-slate-800 rounded-2xl w-full max-w-md p-6 space-y-4 shadow-2xl">
            <div className="flex items-center justify-between pb-3 border-b border-slate-800">
              <h3 className="text-sm font-bold text-white flex items-center gap-2">
                <ArrowDownCircle className="w-4 h-4 text-emerald-400" />
                Simulated Deposit (Virtual Money)
              </h3>
              <button
                onClick={() => setShowDepositModal(false)}
                className="text-slate-400 hover:text-white text-xs font-bold"
              >
                ✕
              </button>
            </div>

            <p className="text-xs text-slate-400">
              This is simulated paper trading. No real bank accounts or actual money are involved.
            </p>

            <form onSubmit={handleDeposit} className="space-y-4">
              <div>
                <label className="block text-xs font-medium text-slate-400 mb-1">
                  Deposit Amount (₹)
                </label>
                <input
                  type="number"
                  min="1000"
                  step="1000"
                  value={depositAmount}
                  onChange={(e) => setDepositAmount(parseFloat(e.target.value) || 0)}
                  className="w-full px-3 py-2 bg-slate-950 border border-slate-800 rounded-lg text-white font-mono text-base font-bold focus:outline-none focus:border-blue-500"
                />
              </div>

              <div className="flex gap-2">
                {[10000, 50000, 100000, 500000].map((amt) => (
                  <button
                    key={amt}
                    type="button"
                    onClick={() => setDepositAmount(amt)}
                    className="flex-1 py-1 text-xs bg-slate-800 hover:bg-slate-700 text-slate-300 rounded font-mono"
                  >
                    +₹{(amt / 1000).toFixed(0)}k
                  </button>
                ))}
              </div>

              <button
                type="submit"
                disabled={submitting}
                className="w-full py-2.5 bg-blue-600 hover:bg-blue-500 text-white rounded-xl text-xs font-bold shadow-lg shadow-blue-600/30 transition-all disabled:opacity-50"
              >
                {submitting ? 'Depositing...' : 'Confirm Virtual Deposit'}
              </button>
            </form>
          </div>
        </div>
      )}

      {/* Withdraw Funds Modal */}
      {showWithdrawModal && (
        <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-slate-950/80 backdrop-blur-xs">
          <div className="bg-slate-900 border border-slate-800 rounded-2xl w-full max-w-md p-6 space-y-4 shadow-2xl">
            <div className="flex items-center justify-between pb-3 border-b border-slate-800">
              <h3 className="text-sm font-bold text-white flex items-center gap-2">
                <ArrowUpCircle className="w-4 h-4 text-amber-400" />
                Simulated Withdrawal
              </h3>
              <button
                onClick={() => setShowWithdrawModal(false)}
                className="text-slate-400 hover:text-white text-xs font-bold"
              >
                ✕
              </button>
            </div>

            <form onSubmit={handleWithdraw} className="space-y-4">
              <div>
                <label className="block text-xs font-medium text-slate-400 mb-1">
                  Withdrawal Amount (₹)
                </label>
                <input
                  type="number"
                  min="500"
                  max={withdrawable}
                  step="500"
                  value={withdrawAmount}
                  onChange={(e) => setWithdrawAmount(parseFloat(e.target.value) || 0)}
                  className="w-full px-3 py-2 bg-slate-950 border border-slate-800 rounded-lg text-white font-mono text-base font-bold focus:outline-none focus:border-blue-500"
                />
                <div className="text-[11px] text-slate-500 mt-1">
                  Max Withdrawable: ₹{withdrawable.toLocaleString('en-IN', { minimumFractionDigits: 2 })}
                </div>
              </div>

              <button
                type="submit"
                disabled={submitting || withdrawAmount > withdrawable}
                className="w-full py-2.5 bg-rose-600 hover:bg-rose-500 text-white rounded-xl text-xs font-bold shadow-lg shadow-rose-600/30 transition-all disabled:opacity-50"
              >
                {submitting ? 'Processing...' : 'Confirm Simulated Withdrawal'}
              </button>
            </form>
          </div>
        </div>
      )}
    </div>
  );
};
