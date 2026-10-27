import React, { createContext, useContext, useState, useEffect, useCallback } from 'react';
import { Stock, MarketOverview } from '../types';
import { stockApi } from '../services/api';
import { wsClient, WebSocketMessage } from '../services/websocket';

interface MarketContextType {
  stocks: Stock[];
  stockMap: Record<string, Stock>;
  marketOverview: MarketOverview | null;
  marketOpen: boolean;
  priceFlashes: Record<string, 'up' | 'down'>;
  refreshMarketData: () => Promise<void>;
  orderModalState: {
    isOpen: boolean;
    stock: Stock | null;
    initialType: 'BUY' | 'SELL';
  };
  openOrderModal: (stock: Stock, type?: 'BUY' | 'SELL') => void;
  closeOrderModal: () => void;
}

const MarketContext = createContext<MarketContextType>({
  stocks: [],
  stockMap: {},
  marketOverview: null,
  marketOpen: true,
  priceFlashes: {},
  refreshMarketData: async () => {},
  orderModalState: { isOpen: false, stock: null, initialType: 'BUY' },
  openOrderModal: () => {},
  closeOrderModal: () => {},
});

export const MarketProvider: React.FC<{ children: React.ReactNode }> = ({ children }) => {
  const [stocks, setStocks] = useState<Stock[]>([]);
  const [stockMap, setStockMap] = useState<Record<string, Stock>>({});
  const [marketOverview, setMarketOverview] = useState<MarketOverview | null>(null);
  const [marketOpen, setMarketOpen] = useState<boolean>(true);
  const [priceFlashes, setPriceFlashes] = useState<Record<string, 'up' | 'down'>>({});
  const [orderModalState, setOrderModalState] = useState<{
    isOpen: boolean;
    stock: Stock | null;
    initialType: 'BUY' | 'SELL';
  }>({
    isOpen: false,
    stock: null,
    initialType: 'BUY',
  });

  const openOrderModal = useCallback((stock: Stock, type: 'BUY' | 'SELL' = 'BUY') => {
    setOrderModalState({
      isOpen: true,
      stock,
      initialType: type,
    });
  }, []);

  const closeOrderModal = useCallback(() => {
    setOrderModalState((prev) => ({ ...prev, isOpen: false }));
  }, []);

  const refreshMarketData = useCallback(async () => {
    try {
      const [stocksRes, ovRes] = await Promise.all([
        stockApi.getAll(),
        stockApi.getMarketOverview(),
      ]);

      if (stocksRes.data && stocksRes.data.data) {
        const list = stocksRes.data.data;
        setStocks(list);
        const map: Record<string, Stock> = {};
        list.forEach((s) => {
          map[s.symbol] = s;
        });
        setStockMap(map);
      }

      if (ovRes.data && ovRes.data.data) {
        setMarketOverview(ovRes.data.data);
        setMarketOpen(ovRes.data.data.marketOpen);
      }
    } catch (e) {
      console.error('[MarketContext] Failed to load market data', e);
    }
  }, []);

  useEffect(() => {
    refreshMarketData();
    wsClient.connect();

    const unsubscribe = wsClient.subscribe((msg: WebSocketMessage) => {
      if (msg.type === 'PRICE_TICK') {
        const { symbol, price, change, changePercent, dayHigh, dayLow, volume } = msg;

        setStocks((prev) =>
          prev.map((s) => {
            if (s.symbol === symbol) {
              const direction = price > s.currentPrice ? 'up' : 'down';
              setPriceFlashes((f) => ({ ...f, [symbol]: direction }));
              setTimeout(() => {
                setPriceFlashes((f) => {
                  const copy = { ...f };
                  delete copy[symbol];
                  return copy;
                });
              }, 1200);

              return {
                ...s,
                currentPrice: price,
                change,
                changePercent,
                dayHigh,
                dayLow,
                volume,
              };
            }
            return s;
          })
        );

        setStockMap((prev) => {
          if (!prev[symbol]) return prev;
          return {
            ...prev,
            [symbol]: {
              ...prev[symbol],
              currentPrice: price,
              change,
              changePercent,
              dayHigh,
              dayLow,
              volume,
            },
          };
        });
      }
    });

    return () => {
      unsubscribe();
    };
  }, [refreshMarketData]);

  return (
    <MarketContext.Provider
      value={{
        stocks,
        stockMap,
        marketOverview,
        marketOpen,
        priceFlashes,
        refreshMarketData,
        orderModalState,
        openOrderModal,
        closeOrderModal,
      }}
    >
      {children}
    </MarketContext.Provider>
  );
};

export const useMarket = () => useContext(MarketContext);
