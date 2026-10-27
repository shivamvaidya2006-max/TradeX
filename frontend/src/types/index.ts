export interface User {
  id: number;
  username: string;
  fullName: string;
  email: string;
  role: 'USER' | 'ADMIN' | 'SUPER_ADMIN';
  phone?: string;
  status?: string;
}

export interface Stock {
  id: number;
  symbol: string;
  companyName: string;
  exchange: string;
  sector: string;
  currentPrice: number;
  previousClose: number;
  openPrice: number;
  dayHigh: number;
  dayLow: number;
  volume: number;
  marketCap: number;
  change: number;
  changePercent: number;
}

export interface Watchlist {
  id: number;
  name: string;
  items: Stock[];
}

export interface Order {
  id: number;
  symbol: string;
  companyName?: string;
  orderType: 'MARKET' | 'LIMIT' | 'STOP_LOSS';
  transactionType: 'BUY' | 'SELL';
  quantity: number;
  price: number;
  triggerPrice: number;
  status: 'PENDING' | 'EXECUTED' | 'CANCELLED' | 'REJECTED';
  createdAt: string;
  executedAt?: string;
}

export interface Holding {
  id: number;
  symbol: string;
  companyName: string;
  quantity: number;
  averagePrice: number;
  currentPrice: number;
  investedValue: number;
  currentValue: number;
  pnl: number;
  pnlPercent: number;
}

export interface Position {
  id: number;
  symbol: string;
  companyName: string;
  quantity: number;
  averagePrice: number;
  currentPrice: number;
  realizedPnl: number;
  unrealizedPnl: number;
  totalPnl: number;
}

export interface Wallet {
  availableBalance: number;
  investedAmount: number;
  withdrawableAmount: number;
  totalAccountValue: number;
}

export interface PortfolioSummary {
  totalInvested: number;
  totalCurrentValue: number;
  totalPnl: number;
  totalPnlPercent: number;
  todayPnl: number;
  todayPnlPercent: number;
}

export interface Transaction {
  id: number;
  type: 'BUY' | 'SELL' | 'DEPOSIT' | 'WITHDRAWAL';
  amount: number;
  description: string;
  createdAt: string;
}

export interface NotificationItem {
  id: number;
  title: string;
  message: string;
  type: 'ORDER' | 'WALLET' | 'MARKET' | 'SYSTEM';
  isRead: boolean;
  createdAt: string;
}

export interface PriceHistoryPoint {
  timestamp: string;
  open: number;
  high: number;
  low: number;
  close: number;
  volume: number;
}

export interface MarketOverview {
  nifty50: { price: number; change: number; changePercent: number };
  bankNifty: { price: number; change: number; changePercent: number };
  sensex: { price: number; change: number; changePercent: number };
  topGainers: Stock[];
  topLosers: Stock[];
  mostActive: Stock[];
  marketOpen: boolean;
}

// DSA Types
export interface TrieVisualNode {
  character: string;
  isTerminal: boolean;
  symbol: string;
  children: TrieVisualNode[];
}

export interface OrderBookLevel {
  price: number;
  quantity: number;
  orderCount: number;
}

export interface OrderBookData {
  symbol: string;
  ltp: number;
  spread: number;
  bids: OrderBookLevel[];
  asks: OrderBookLevel[];
  heapStructure: {
    buySide: string;
    sellSide: string;
  };
}

export interface BSTData {
  algorithm: string;
  timeComplexity: string;
  spaceComplexity: string;
  inorder: number[];
  preorder: number[];
  postorder: number[];
  minPrice: number;
  maxPrice: number;
}

export interface GraphNode {
  id: string;
  label: string;
  type: 'STOCK' | 'SECTOR';
  price: number;
}

export interface GraphEdge {
  source: string;
  target: string;
  relationship: string;
}

export interface SortStep {
  arrayState: number[];
  comparedIndex1: number;
  comparedIndex2: number;
  swappedIndex1: number;
  swappedIndex2: number;
  description: string;
}

export interface SortResult {
  algorithm: string;
  timeComplexity: string;
  spaceComplexity: string;
  comparisons: number;
  swaps: number;
  durationMicroseconds: number;
  sortedArray: number[];
  steps: SortStep[];
}

export interface SearchStep {
  index: number;
  low: number;
  mid: number;
  high: number;
  value: number;
  action: string;
}

export interface SearchResult {
  algorithm: string;
  target: number;
  foundIndex: number;
  comparisons: number;
  durationMicroseconds: number;
  timeComplexity: string;
  spaceComplexity: string;
  steps: SearchStep[];
}

export interface BestWindow {
  startIndex: number;
  endIndex: number;
  startPrice: number;
  endPrice: number;
  absoluteGain: number;
  percentageGain: number;
  timeComplexity: string;
}

export interface ConsecutiveStreak {
  startIndex: number;
  endIndex: number;
  consecutiveDays: number;
  totalGain: number;
  timeComplexity: string;
}

export interface SlidingWindowResult {
  symbol: string;
  windowSize: number;
  bestWindow: BestWindow;
  maxConsecutiveGainStreak: ConsecutiveStreak;
}

export interface AllocationItem {
  symbol: string;
  companyName: string;
  currentPrice: number;
  shares: number;
  investedAmount: number;
  portfolioWeight: number;
  expectedReturn: number;
  riskScore: number;
}

export interface GreedyAllocationResult {
  totalCapital: number;
  capitalAllocated: number;
  remainingCash: number;
  weightedExpectedReturn: number;
  averageRiskScore: number;
  allocations: AllocationItem[];
  complexity: string;
  disclaimer: string;
}

export interface DPProfitResult {
  symbol: string;
  singleTransaction: {
    maxProfit: number;
    buyDayIndex: number;
    sellDayIndex: number;
    buyPrice: number;
    sellPrice: number;
    timeComplexity: string;
    spaceComplexity: string;
  };
  unlimitedTransactions: {
    maxProfit: number;
    timeComplexity: string;
    spaceComplexity: string;
  };
  twoTransactionsDP: {
    maxProfit: number;
    kAllowed: number;
    timeComplexity: string;
    spaceComplexity: string;
  };
}
