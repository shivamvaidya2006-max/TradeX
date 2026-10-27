import React, { useState, useEffect } from 'react';
import { X, AlertCircle, CheckCircle2, TrendingUp, TrendingDown, Info, ShieldCheck } from 'lucide-react';
import { useMarket } from '../context/MarketContext';
import { orderApi, walletApi } from '../services/api';
import { Stock, Wallet } from '../types';

interface OrderModalProps {
  onOrderSuccess?: () => void;
}

export const OrderModal: React.FC<OrderModalProps> = ({ onOrderSuccess }) => {
  const { orderModalState, closeOrderModal, stockMap } = useMarket();
  const { isOpen, stock: initialStock, initialType } = orderModalState;

  const [transactionType, setTransactionType] = useState<'BUY' | 'SELL'>('BUY');
  const [orderType, setOrderType] = useState<'MARKET' | 'LIMIT' | 'STOP_LOSS'>('MARKET');
  const [productType, setProductType] = useState<'CNC' | 'MIS'>('CNC');
  const [quantity, setQuantity] = useState<number>(1);
  const [price, setPrice] = useState<number>(0);
  const [triggerPrice, setTriggerPrice] = useState<number>(0);
  
  const [wallet, setWallet] = useState<Wallet | null>(null);
  const [loading, setLoading] = useState<boolean>(false);
  const [error, setError] = useState<string | null>(null);
  const [success, setSuccess] = useState<string | null>(null);

  // Sync state when modal opens
  useEffect(() => {
    if (isOpen && initialStock) {
      const liveStock = stockMap[initialStock.symbol] || initialStock;
      setTransactionType(initialType);
      setQuantity(1);
      setPrice(liveStock.currentPrice);
      setTriggerPrice(parseFloat((liveStock.currentPrice * 0.98).toFixed(2)));
      setError(null);
      setSuccess(null);
      fetchWallet();
    }
  }, [isOpen, initialStock, initialType, stockMap]);

  const fetchWallet = async () => {
    try {
      const res = await walletApi.getWallet();
      if (res.data && res.data.data) {
        setWallet(res.data.data);
      }
    } catch (e) {
      console.error('Failed to fetch wallet info', e);
    }
  };

  if (!isOpen || !initialStock) return null;

  const currentStock = stockMap[initialStock.symbol] || initialStock;
  const executionPrice = orderType === 'MARKET' ? currentStock.currentPrice : price;
  const orderValue = quantity * executionPrice;

  // Real-world simulated Indian brokerage charges:
  // Brokerage: Flat ₹20 or 0.03% (whichever is lower)
  const brokerage = Math.min(20, parseFloat((orderValue * 0.0003).toFixed(2)));
  // STT: 0.1% on delivery (CNC)
  const stt = productType === 'CNC' ? parseFloat((orderValue * 0.001).toFixed(2)) : 0;
  // Exchange txn fee: 0.00345%
  const exchangeTxn = parseFloat((orderValue * 0.0000345).toFixed(2));
  // GST: 18% on (brokerage + exchange txn)
  const gst = parseFloat(((brokerage + exchangeTxn) * 0.18).toFixed(2));
  // Stamp duty: 0.015% on buy only
  const stampDuty = transactionType === 'BUY' ? parseFloat((orderValue * 0.00015).toFixed(2)) : 0;
  const totalCharges = parseFloat((brokerage + stt + exchangeTxn + gst + stampDuty).toFixed(2));
  
  const totalRequired = transactionType === 'BUY' 
    ? parseFloat((orderValue + totalCharges).toFixed(2))
    : parseFloat((orderValue - totalCharges).toFixed(2));

  const availableBalance = wallet?.availableBalance || 0;
  const hasInsufficientFunds = transactionType === 'BUY' && totalRequired > availableBalance;

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    setError(null);
    setSuccess(null);

    if (quantity <= 0) {
      setError('Quantity must be greater than 0');
      return;
    }

    if (orderType !== 'MARKET' && price <= 0) {
      setError('Limit price must be greater than 0');
      return;
    }

    if (orderType === 'STOP_LOSS' && triggerPrice <= 0) {
      setError('Trigger price must be greater than 0');
      return;
    }

    if (hasInsufficientFunds) {
      setError(`Insufficient available balance! Required: ₹${totalRequired.toLocaleString('en-IN')}, Available: ₹${availableBalance.toLocaleString('en-IN')}`);
      return;
    }

    try {
      setLoading(true);
      const res = await orderApi.placeOrder({
        symbol: currentStock.symbol,
        transactionType,
        orderType,
        quantity,
        price: orderType === 'MARKET' ? currentStock.currentPrice : price,
        triggerPrice: orderType === 'STOP_LOSS' ? triggerPrice : 0,
      });

      if (res.data) {
        setSuccess(`Order placed successfully! (${transactionType} ${quantity} ${currentStock.symbol})`);
        if (onOrderSuccess) {
          onOrderSuccess();
        }
        setTimeout(() => {
          closeOrderModal();
        }, 1200);
      }
    } catch (err: any) {
      const errMsg = err.response?.data?.error || err.message || 'Failed to place order';
      setError(errMsg);
    } finally {
      setLoading(false);
    }
  };

  const isBuy = transactionType === 'BUY';
  const themeColor = isBuy ? 'blue' : 'rose';

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-slate-950/80 backdrop-blur-sm animate-in fade-in duration-200">
      <div 
        className="w-full max-w-lg bg-slate-900 border border-slate-800 rounded-2xl shadow-2xl overflow-hidden flex flex-col max-h-[92vh]"
        onClick={(e) => e.stopPropagation()}
      >
        {/* Header */}
        <div className={`p-4 ${isBuy ? 'bg-blue-600/10 border-b border-blue-500/20' : 'bg-rose-600/10 border-b border-rose-500/20'} flex items-center justify-between`}>
          <div>
            <div className="flex items-center space-x-2">
              <span className={`px-2.5 py-0.5 rounded text-xs font-bold uppercase tracking-wider ${isBuy ? 'bg-blue-500 text-white' : 'bg-rose-500 text-white'}`}>
                {transactionType}
              </span>
              <h2 className="text-lg font-bold text-white tracking-tight">
                {currentStock.symbol}
              </h2>
              <span className="text-xs text-slate-400">
                {currentStock.exchange}
              </span>
            </div>
            <div className="flex items-center space-x-2 mt-1">
              <span className="text-base font-semibold text-slate-200">
                ₹{currentStock.currentPrice.toLocaleString('en-IN', { minimumFractionDigits: 2 })}
              </span>
              <span className={`text-xs flex items-center font-medium ${currentStock.change >= 0 ? 'text-emerald-400' : 'text-rose-400'}`}>
                {currentStock.change >= 0 ? '+' : ''}{currentStock.change.toFixed(2)} ({currentStock.changePercent.toFixed(2)}%)
              </span>
            </div>
          </div>

          <button
            onClick={closeOrderModal}
            className="p-1.5 rounded-lg text-slate-400 hover:text-white hover:bg-slate-800 transition-colors"
          >
            <X className="w-5 h-5" />
          </button>
        </div>

        {/* Form Body */}
        <form onSubmit={handleSubmit} className="p-5 overflow-y-auto space-y-4">
          {/* Buy / Sell Tabs */}
          <div className="grid grid-cols-2 gap-2 bg-slate-950 p-1 rounded-xl border border-slate-800">
            <button
              type="button"
              onClick={() => setTransactionType('BUY')}
              className={`py-2 text-sm font-semibold rounded-lg transition-all ${
                isBuy
                  ? 'bg-blue-600 text-white shadow-lg shadow-blue-600/30'
                  : 'text-slate-400 hover:text-white'
              }`}
            >
              BUY
            </button>
            <button
              type="button"
              onClick={() => setTransactionType('SELL')}
              className={`py-2 text-sm font-semibold rounded-lg transition-all ${
                !isBuy
                  ? 'bg-rose-600 text-white shadow-lg shadow-rose-600/30'
                  : 'text-slate-400 hover:text-white'
              }`}
            >
              SELL
            </button>
          </div>

          {/* Product Type (CNC / MIS) */}
          <div className="space-y-1.5">
            <label className="text-xs font-medium text-slate-400 uppercase tracking-wider">Product</label>
            <div className="grid grid-cols-2 gap-2">
              <button
                type="button"
                onClick={() => setProductType('CNC')}
                className={`py-2 px-3 text-xs font-semibold rounded-lg border text-left transition-all ${
                  productType === 'CNC'
                    ? 'border-blue-500/50 bg-blue-500/10 text-white'
                    : 'border-slate-800 bg-slate-950/50 text-slate-400 hover:border-slate-700'
                }`}
              >
                <div className="font-bold">CNC (Longterm)</div>
                <div className="text-[10px] text-slate-400">Cash & Carry / Delivery</div>
              </button>
              <button
                type="button"
                onClick={() => setProductType('MIS')}
                className={`py-2 px-3 text-xs font-semibold rounded-lg border text-left transition-all ${
                  productType === 'MIS'
                    ? 'border-blue-500/50 bg-blue-500/10 text-white'
                    : 'border-slate-800 bg-slate-950/50 text-slate-400 hover:border-slate-700'
                }`}
              >
                <div className="font-bold">MIS (Intraday)</div>
                <div className="text-[10px] text-slate-400">Square off before 15:20</div>
              </button>
            </div>
          </div>

          {/* Order Type (Market / Limit / SL) */}
          <div className="space-y-1.5">
            <label className="text-xs font-medium text-slate-400 uppercase tracking-wider">Order Type</label>
            <div className="grid grid-cols-3 gap-2">
              {(['MARKET', 'LIMIT', 'STOP_LOSS'] as const).map((t) => (
                <button
                  key={t}
                  type="button"
                  onClick={() => setOrderType(t)}
                  className={`py-2 text-xs font-semibold rounded-lg border transition-all ${
                    orderType === t
                      ? 'border-blue-500/50 bg-blue-500/10 text-white'
                      : 'border-slate-800 bg-slate-950/50 text-slate-400 hover:border-slate-700'
                  }`}
                >
                  {t === 'STOP_LOSS' ? 'SL' : t}
                </button>
              ))}
            </div>
          </div>

          {/* Quantity & Price Row */}
          <div className="grid grid-cols-2 gap-3">
            <div>
              <label className="block text-xs font-medium text-slate-400 mb-1">Quantity</label>
              <input
                type="number"
                min="1"
                step="1"
                value={quantity}
                onChange={(e) => setQuantity(Math.max(1, parseInt(e.target.value) || 1))}
                className="w-full px-3 py-2 bg-slate-950 border border-slate-800 rounded-lg text-white font-medium focus:outline-none focus:border-blue-500"
              />
              <div className="flex gap-1 mt-1.5">
                {[1, 5, 10, 50, 100].map((q) => (
                  <button
                    key={q}
                    type="button"
                    onClick={() => setQuantity(q)}
                    className="px-2 py-0.5 text-[10px] bg-slate-800 hover:bg-slate-700 text-slate-300 rounded"
                  >
                    {q}
                  </button>
                ))}
              </div>
            </div>

            <div>
              <label className="block text-xs font-medium text-slate-400 mb-1">
                Price {orderType === 'MARKET' && <span className="text-[10px] text-slate-500">(Market)</span>}
              </label>
              <input
                type="number"
                step="0.05"
                min="0.05"
                disabled={orderType === 'MARKET'}
                value={orderType === 'MARKET' ? currentStock.currentPrice : price}
                onChange={(e) => setPrice(parseFloat(e.target.value) || 0)}
                className={`w-full px-3 py-2 bg-slate-950 border border-slate-800 rounded-lg text-white font-medium focus:outline-none focus:border-blue-500 ${
                  orderType === 'MARKET' ? 'opacity-60 cursor-not-allowed' : ''
                }`}
              />
            </div>
          </div>

          {/* Trigger Price for SL */}
          {orderType === 'STOP_LOSS' && (
            <div>
              <label className="block text-xs font-medium text-slate-400 mb-1">Trigger Price</label>
              <input
                type="number"
                step="0.05"
                min="0.05"
                value={triggerPrice}
                onChange={(e) => setTriggerPrice(parseFloat(e.target.value) || 0)}
                className="w-full px-3 py-2 bg-slate-950 border border-slate-800 rounded-lg text-white font-medium focus:outline-none focus:border-blue-500"
              />
            </div>
          )}

          {/* Cost Breakdown & Charges Card */}
          <div className="bg-slate-950/70 border border-slate-800/80 rounded-xl p-3.5 space-y-2 text-xs">
            <div className="flex justify-between items-center text-slate-400">
              <span>Gross Order Value:</span>
              <span className="font-semibold text-slate-200">
                ₹{orderValue.toLocaleString('en-IN', { minimumFractionDigits: 2, maximumFractionDigits: 2 })}
              </span>
            </div>

            <div className="flex justify-between items-center text-slate-400">
              <span className="flex items-center space-x-1">
                <span>Simulated Charges:</span>
                <span title="Brokerage ₹20 cap, STT, Exchange, GST, Stamp" className="text-slate-500 cursor-help">ⓘ</span>
              </span>
              <span className="text-slate-300">
                + ₹{totalCharges.toFixed(2)}
              </span>
            </div>

            <div className="pt-2 border-t border-slate-800 flex justify-between items-center">
              <span className="font-medium text-slate-300">
                {isBuy ? 'Margin Required:' : 'Net Receivable:'}
              </span>
              <span className={`font-bold text-sm ${isBuy ? 'text-blue-400' : 'text-emerald-400'}`}>
                ₹{totalRequired.toLocaleString('en-IN', { minimumFractionDigits: 2, maximumFractionDigits: 2 })}
              </span>
            </div>

            <div className="flex justify-between items-center pt-1 text-[11px] text-slate-400">
              <span>Available Cash:</span>
              <span className={`font-semibold ${hasInsufficientFunds ? 'text-rose-400' : 'text-slate-300'}`}>
                ₹{availableBalance.toLocaleString('en-IN', { minimumFractionDigits: 2 })}
              </span>
            </div>
          </div>

          {/* Messages */}
          {error && (
            <div className="p-3 bg-rose-500/10 border border-rose-500/20 rounded-xl flex items-start space-x-2 text-rose-400 text-xs">
              <AlertCircle className="w-4 h-4 shrink-0 mt-0.5" />
              <span>{error}</span>
            </div>
          )}

          {success && (
            <div className="p-3 bg-emerald-500/10 border border-emerald-500/20 rounded-xl flex items-center space-x-2 text-emerald-400 text-xs">
              <CheckCircle2 className="w-4 h-4 shrink-0" />
              <span>{success}</span>
            </div>
          )}

          {/* Action Button */}
          <button
            type="submit"
            disabled={loading || (isBuy && hasInsufficientFunds)}
            className={`w-full py-3 rounded-xl font-bold text-sm text-white shadow-lg transition-all flex items-center justify-center space-x-2 ${
              isBuy
                ? 'bg-blue-600 hover:bg-blue-500 shadow-blue-600/30'
                : 'bg-rose-600 hover:bg-rose-500 shadow-rose-600/30'
            } disabled:opacity-50 disabled:cursor-not-allowed`}
          >
            {loading ? (
              <span>Processing Order...</span>
            ) : (
              <>
                <span>{transactionType} {quantity} {currentStock.symbol}</span>
                <span className="text-xs opacity-75">(@ ₹{executionPrice.toFixed(2)})</span>
              </>
            )}
          </button>
        </form>
      </div>
    </div>
  );
};
