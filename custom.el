(custom-set-variables
 '(font-lock-mode t nil (font-lock))
 '(paren-mode (quote sexp) t))
(custom-set-faces
 '(default ((t (:size "13pt" :family "Lucidatypewriter"))) t))
(add-to-list 'auto-mode-alist '("\\.kc" . c-mode))
(setq column-number-mode t)
(setq line-number-mode t)
(add-to-list 'default-frame-alist '(fullscreen . maximized))
